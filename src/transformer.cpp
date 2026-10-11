#include "transformer.hpp"
#include <tree_sitter/api.h>
#include <iostream>
#include <algorithm>
#include <cstdint>
#include <utility>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <string>
#include <vector>

extern "C" const TSLanguage* tree_sitter_cpp(void);

namespace fs = std::filesystem;

namespace {

struct Edit {
    std::size_t start;
    std::size_t end;
    std::string replacement;
};

struct Candidate {
    TSNode declaration;
    TSNode nameNode;
    std::string name;
};

std::string nodeText(TSNode node, const std::string& source) {
    const std::size_t start = ts_node_start_byte(node);
    const std::size_t end = ts_node_end_byte(node);

    if (start > end || end > source.size()) {
        return {};
    }

    return source.substr(start, end - start);
}

bool isSimpleInitializedLocal(TSNode declaration, Candidate& result,
                              const std::string& source) {
    if (std::string(ts_node_type(declaration)) != "declaration") {
        return false;
    }

    bool hasPrimitiveType = false;
    TSNode initDeclarator = {};
    int initDeclaratorCount = 0;

    const std::uint32_t count = ts_node_child_count(declaration);

    for (std::uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(declaration, i);
        const std::string type = ts_node_type(child);

        if (type == "primitive_type") {
            hasPrimitiveType = true;
        }

        if (type == "init_declarator") {
            initDeclarator = child;
            ++initDeclaratorCount;
        }
    }

    // Initially support only one initialized variable with a built-in type.
    if (!hasPrimitiveType || initDeclaratorCount != 1) {
        return false;
    }

    TSNode nameNode = ts_node_child_by_field_name(
        initDeclarator, "declarator", 10
    );

    if (ts_node_is_null(nameNode) ||
        std::string(ts_node_type(nameNode)) != "identifier") {
        return false;
    }

    result.declaration = declaration;
    result.nameNode = nameNode;
    result.name = nodeText(nameNode, source);

    return !result.name.empty();
}

bool subtreeContainsName(TSNode node, const std::string& name,
                         const std::string& source) {
    if (std::string(ts_node_type(node)) == "identifier" &&
        nodeText(node, source) == name) {
        return true;
    }

    const std::uint32_t count = ts_node_child_count(node);

    for (std::uint32_t i = 0; i < count; ++i) {
        if (subtreeContainsName(ts_node_child(node, i), name, source)) {
            return true;
        }
    }

    return false;
}

bool isAnotherDeclarationName(TSNode node) {
    TSNode parent = ts_node_parent(node);

    if (ts_node_is_null(parent)) {
        return false;
    }

    const std::string parentType = ts_node_type(parent);

    if (parentType == "init_declarator" ||
        parentType == "parameter_declaration" ||
        parentType == "optional_parameter_declaration" ||
        parentType == "condition_clause") {
        TSNode declarator = ts_node_child_by_field_name(
            parent, "declarator", 10
        );

        if (!ts_node_is_null(declarator) &&
            ts_node_eq(declarator, node)) {
            return true;
        }
    }

    if (parentType == "declaration") {
        TSNode declarator = ts_node_child_by_field_name(
            parent, "declarator", 10
        );

        if (!ts_node_is_null(declarator) &&
            ts_node_eq(declarator, node)) {
            return true;
        }
    }

    return false;
}

void collectReferences(
    TSNode node,
    TSNode rootBody,
    const Candidate& candidate,
    const std::string& source,
    std::vector<TSNode>& references,
    bool& unsafe
) {
    const std::string type = ts_node_type(node);

    // Don't descend into nested blocks: their scopes need separate analysis.
    if (!ts_node_eq(node, rootBody) && type == "compound_statement") {
        if (subtreeContainsName(node, candidate.name, source)) {
            unsafe = true;
        }
        return;
    }

    if (type == "identifier" &&
        nodeText(node, source) == candidate.name) {

        if (ts_node_eq(node, candidate.nameNode)) {
            // The declaration itself is handled separately.
        } else if (isAnotherDeclarationName(node)) {
            // Same spelling may refer to a shadowing declaration.
            unsafe = true;
        } else {
            references.push_back(node);
        }
    }

    const std::uint32_t count = ts_node_child_count(node);

    for (std::uint32_t i = 0; i < count && !unsafe; ++i) {
        collectReferences(
            ts_node_child(node, i),
            rootBody,
            candidate,
            source,
            references,
            unsafe
        );
    }
}

void findCandidates(
    TSNode node,
    const std::string& source,
    std::vector<Candidate>& candidates
) {
    if (std::string(ts_node_type(node)) == "function_definition") {
        TSNode body = ts_node_child_by_field_name(node, "body", 4);

        if (!ts_node_is_null(body) &&
            std::string(ts_node_type(body)) == "compound_statement") {

            const std::uint32_t count = ts_node_child_count(body);

            // Only inspect declarations directly inside the function body.
            for (std::uint32_t i = 0; i < count; ++i) {
                TSNode child = ts_node_child(body, i);

                Candidate candidate{};

                if (isSimpleInitializedLocal(child, candidate, source)) {
                    candidates.push_back(candidate);
                }
            }
        }
    }

    const std::uint32_t count = ts_node_child_count(node);

    for (std::uint32_t i = 0; i < count; ++i) {
        findCandidates(ts_node_child(node, i), source, candidates);
    }
}

} // namespace



static void collectCommentRanges(
    TSNode node,
    std::vector<std::pair<uint32_t, uint32_t>>& ranges
) {
    if (std::string(ts_node_type(node)) == "comment") {
        ranges.emplace_back(
            ts_node_start_byte(node),
            ts_node_end_byte(node)
        );
        return;
    }

    uint32_t childCount = ts_node_child_count(node);

    for (uint32_t i = 0; i < childCount; ++i) {
        collectCommentRanges(ts_node_child(node, i), ranges);
    }
}

static bool removeCommentsFromSource(
    std::string& source,
    std::string& error
) {
    TSParser* parser = ts_parser_new();

    if (parser == nullptr) {
        error = "Failed to create Tree-sitter parser for comment removal.";
        return false;
    }

    if (!ts_parser_set_language(parser, tree_sitter_cpp())) {
        ts_parser_delete(parser);
        error = "Failed to set the C++ grammar for comment removal.";
        return false;
    }

    TSTree* tree = ts_parser_parse_string(
        parser,
        nullptr,
        source.data(),
        static_cast<uint32_t>(source.size())
    );

    if (tree == nullptr) {
        ts_parser_delete(parser);
        error = "Failed to parse source during comment removal.";
        return false;
    }

    TSNode root = ts_tree_root_node(tree);

    if (ts_node_has_error(root)) {
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        error = "Source contains syntax errors during comment removal.";
        return false;
    }

    std::vector<std::pair<uint32_t, uint32_t>> ranges;
    collectCommentRanges(root, ranges);

    ts_tree_delete(tree);
    ts_parser_delete(parser);

    // Replace comment characters with spaces, preserving line breaks.
    // This avoids accidentally joining adjacent C++ tokens.
    for (const auto& range : ranges) {
        uint32_t start = range.first;
        uint32_t end = range.second;

        for (uint32_t i = start; i < end && i < source.size(); ++i) {
            if (source[i] != '\n' && source[i] != '\r') {
                source[i] = ' ';
            }
        }
    }

    return true;
}

bool transformCppFile(
    const std::string& inputPath,
    const std::string& outputPath,
    int level,
    std::string& error
) {


    if (level < 1 || level > 3) {
        error = "Invalid obfuscation level.";
        return false;
    }
    std::error_code ec;

    const fs::path inputAbsolute = fs::absolute(inputPath, ec).lexically_normal();

    if (ec) {
        error = "Could not resolve input path.";
        return false;
    }

    const fs::path outputAbsolute = fs::absolute(outputPath, ec).lexically_normal();

    if (ec) {
        error = "Could not resolve output path.";
        return false;
    }

    if (inputAbsolute == outputAbsolute) {
        error = "Input and output paths must be different.";
        return false;
    }

    std::ifstream input(inputAbsolute, std::ios::binary);

    if (!input) {
        error = "Could not open input file for transformation.";
        return false;
    }

    

    std::string source{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    

    if (input.bad()) {
        error = "Failed while reading input file.";
        return false;
    }

    if (source.size() > std::numeric_limits<std::uint32_t>::max()) {
        error = "Input file is too large for this implementation.";
        return false;
    }

    TSParser* parser = ts_parser_new();

    if (parser == nullptr) {
        error = "Could not create parser for transformation.";
        return false;
    }

    if (!ts_parser_set_language(parser, tree_sitter_cpp())) {
        ts_parser_delete(parser);
        error = "Could not load the C++ grammar.";
        return false;
    }

    TSTree* tree = ts_parser_parse_string(
        parser,
        nullptr,
        source.data(),
        static_cast<std::uint32_t>(source.size())
    );

    if (tree == nullptr) {
        ts_parser_delete(parser);
        error = "Could not parse source for transformation.";
        return false;
    }

    TSNode root = ts_tree_root_node(tree);

    if (ts_node_has_error(root)) {
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        error = "Refusing to transform source containing syntax errors.";
        return false;
    }

    std::vector<Candidate> candidates;
    findCandidates(root, source, candidates);

    std::vector<Edit> edits;
    std::size_t renamedCount = 0;
    std::size_t skippedCount = 0;
    std::size_t sequence = 0;

    for (const Candidate& candidate : candidates) {
        TSNode function = ts_node_parent(candidate.declaration);

        // Find the function body containing this declaration.
        while (!ts_node_is_null(function) &&
               std::string(ts_node_type(function)) != "function_definition") {
            function = ts_node_parent(function);
        }

        if (ts_node_is_null(function)) {
            ++skippedCount;
            continue;
        }

        TSNode body = ts_node_child_by_field_name(function, "body", 4);

        std::vector<TSNode> references;
        bool unsafe = false;

        collectReferences(
            body, body, candidate, source, references, unsafe
        );

        if (unsafe) {
            ++skippedCount;
            continue;
        }

        const std::string replacement =
            "obf_var_" + std::to_string(++sequence);

        edits.push_back({
            static_cast<std::size_t>(ts_node_start_byte(candidate.nameNode)),
            static_cast<std::size_t>(ts_node_end_byte(candidate.nameNode)),
            replacement
        });

        for (TSNode reference : references) {
            edits.push_back({
                static_cast<std::size_t>(ts_node_start_byte(reference)),
                static_cast<std::size_t>(ts_node_end_byte(reference)),
                replacement
            });
        }

        ++renamedCount;
    }

    

    // Apply edits right-to-left so earlier byte offsets remain valid.
    std::sort(edits.begin(), edits.end(),
        [](const Edit& a, const Edit& b) {
            return a.start > b.start;
        });

    for (const Edit& edit : edits) {
        source.replace(
            edit.start,
            edit.end - edit.start,
            edit.replacement
        );
    }

    if (level >= 2) {
        if (!removeCommentsFromSource(source, error)) {
            return false;
        }

        std::cout << "[INFO] Comment removal completed.\n";
    }

    ts_tree_delete(tree);
    ts_parser_delete(parser);

    std::ofstream output(outputAbsolute, std::ios::binary | std::ios::trunc);

    if (!output) {
        error = "Could not open output file for writing.";
        return false;
    }

    output.write(source.data(), static_cast<std::streamsize>(source.size()));

    if (!output) {
        error = "Failed while writing transformed source.";
        return false;
    }

    output.close();

    if (!output) {
        error = "Failed while closing transformed output file.";
        return false;
    }

    std::cout << "[INFO] Local variables renamed: " << renamedCount << '\n';
    std::cout << "[INFO] Variables skipped for safety: " << skippedCount << '\n';
    std::cout << "[INFO] Output written to: " << outputAbsolute.string() << '\n';

    return true;
}