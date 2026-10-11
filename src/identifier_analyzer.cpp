#include "identifier_analyzer.hpp"

#include <tree_sitter/api.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <string>

extern "C" const TSLanguage* tree_sitter_cpp(void);

namespace {

constexpr std::size_t MAX_IDENTIFIERS_TO_PRINT = 200;

bool isIdentifierNode(const char* type) {
    const std::string nodeType = type;

    return nodeType == "identifier"
        || nodeType == "field_identifier"
        || nodeType == "type_identifier"
        || nodeType == "namespace_identifier"
        || nodeType == "namespace_alias_identifier"
        || nodeType == "statement_identifier";
}

void visitNode(
    TSNode node,
    const std::string& source,
    std::size_t& count
) {
    const char* type = ts_node_type(node);

    if (isIdentifierNode(type)) {
        if (count >= MAX_IDENTIFIERS_TO_PRINT) {
            return;
        }

        const std::uint32_t start = ts_node_start_byte(node);
        const std::uint32_t end = ts_node_end_byte(node);

        if (end <= source.size() && start <= end) {
            const std::string name =
                source.substr(start, end - start);

            TSPoint position = ts_node_start_point(node);
            TSNode parent = ts_node_parent(node);

            std::cout
                << "[IDENTIFIER] "
                << name
                << " | node=" << type
                << " | parent=" << ts_node_type(parent)
                << " | line=" << position.row + 1
                << " | column=" << position.column + 1
                << '\n';

            ++count;
        }
    }

    const std::uint32_t children = ts_node_child_count(node);

    for (std::uint32_t i = 0; i < children; ++i) {
        if (count >= MAX_IDENTIFIERS_TO_PRINT) {
            return;
        }

        visitNode(ts_node_child(node, i), source, count);
    }
}

} // namespace

bool analyzeIdentifiers(
    const std::string& inputPath,
    std::string& error
) {
    std::ifstream input(inputPath, std::ios::binary);

    if (!input) {
        error = "Could not open file for identifier analysis: "
              + inputPath;
        return false;
    }

    std::string source{
        std::istreambuf_iterator<char>(input),
        std::istreambuf_iterator<char>()
    };

    if (input.bad()) {
        error = "Failed while reading file for identifier analysis.";
        return false;
    }

    if (source.size() >
        std::numeric_limits<std::uint32_t>::max()) {
        error = "Input file is too large for identifier analysis.";
        return false;
    }

    TSParser* parser = ts_parser_new();

    if (parser == nullptr) {
        error = "Could not create parser for identifier analysis.";
        return false;
    }

    if (!ts_parser_set_language(parser, tree_sitter_cpp())) {
        ts_parser_delete(parser);
        error = "Could not load C++ grammar for identifier analysis.";
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
        error = "Could not parse source for identifier analysis.";
        return false;
    }

    TSNode root = ts_tree_root_node(tree);

    if (ts_node_has_error(root)) {
        error = "Cannot analyze identifiers reliably: "
                "source contains syntax errors.";
        ts_tree_delete(tree);
        ts_parser_delete(parser);
        return false;
    }

    std::cout << "\n[INFO] Identifier analysis:\n";

    std::size_t count = 0;
    visitNode(root, source, count);

    if (count == MAX_IDENTIFIERS_TO_PRINT) {
        std::cout << "[INFO] Identifier listing reached its limit of "
                  << MAX_IDENTIFIERS_TO_PRINT << ".\n";
    }

    std::cout << "[INFO] Identifier candidates listed: "
              << count << '\n';

    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return true;
}