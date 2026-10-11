#include "parser.hpp"

#include <tree_sitter/api.h>

#include <cstdint>
#include <fstream>
#include <iostream>
#include <iterator>
#include <limits>
#include <sstream>
#include <string>

extern "C" const TSLanguage* tree_sitter_cpp(void);

namespace {

constexpr std::size_t MAX_TREE_NODES = 250;
constexpr std::uint32_t MAX_SOURCE_SIZE =
    std::numeric_limits<std::uint32_t>::max();

struct TraversalState {
    std::size_t printedNodes = 0;
    bool truncated = false;
};

void printTree(
    TSNode node,
    int depth,
    TraversalState& state
) {
    if (state.printedNodes >= MAX_TREE_NODES) {
        state.truncated = true;
        return;
    }

    ++state.printedNodes;

    const char* type = ts_node_type(node);
    TSPoint start = ts_node_start_point(node);

    std::cout
        << std::string(static_cast<std::size_t>(depth) * 2, ' ')
        << type
        << " [" << start.row + 1
        << ":" << start.column + 1 << "]";

    if (ts_node_is_error(node)) {
        std::cout << "  <ERROR>";
    } else if (ts_node_is_missing(node)) {
        std::cout << "  <MISSING>";
    }

    std::cout << '\n';

    const std::uint32_t childCount =
        ts_node_child_count(node);

    for (std::uint32_t i = 0; i < childCount; ++i) {
        if (state.printedNodes >= MAX_TREE_NODES) {
            state.truncated = true;
            return;
        }

        TSNode child = ts_node_child(node, i);
        printTree(child, depth + 1, state);
    }
}

void reportSyntaxProblems(TSNode node) {
    if (ts_node_is_error(node) || ts_node_is_missing(node)) {
        TSPoint start = ts_node_start_point(node);

        std::cerr
            << "[SYNTAX ERROR] "
            << (ts_node_is_missing(node)
                    ? "Missing "
                    : "Unexpected ")
            << ts_node_type(node)
            << " at line " << start.row + 1
            << ", column " << start.column + 1
            << '\n';
    }

    const std::uint32_t childCount =
        ts_node_child_count(node);

    for (std::uint32_t i = 0; i < childCount; ++i) {
        reportSyntaxProblems(ts_node_child(node, i));
    }
}

} // namespace

bool parseCppFile(
    const std::string& inputPath,
    std::string& error
) {
    std::ifstream input(inputPath, std::ios::binary);

    if (!input) {
        error = "Could not open input file for parsing: " + inputPath;
        return false;
    }

    std::string source{
    std::istreambuf_iterator<char>(input),
    std::istreambuf_iterator<char>()
    };

    if (input.bad()) {
        error = "Failed while reading input file: " + inputPath;
        return false;
    }

    if (source.size() > MAX_SOURCE_SIZE) {
        error = "Input file is too large for this parser implementation.";
        return false;
    }

    TSParser* parser = ts_parser_new();

    if (parser == nullptr) {
        error = "Could not create the Tree-sitter parser.";
        return false;
    }

    if (!ts_parser_set_language(parser, tree_sitter_cpp())) {
        ts_parser_delete(parser);
        error = "Could not load the Tree-sitter C++ grammar.";
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
        error = "Tree-sitter could not produce a syntax tree.";
        return false;
    }

    TSNode root = ts_tree_root_node(tree);

    std::cout << "\n[INFO] Syntax tree (node type and start position):\n";

    TraversalState state;
    printTree(root, 0, state);

    if (state.truncated) {
        std::cout
            << "[INFO] Tree display truncated after "
            << MAX_TREE_NODES << " nodes.\n";
    }

    const bool hasErrors = ts_node_has_error(root);

    if (hasErrors) {
        std::cerr << "\n[INFO] Syntax problems found:\n";
        reportSyntaxProblems(root);
        error = "Tree-sitter detected syntax errors in the input.";
    } else {
        std::cout << "\n[INFO] C++ syntax parsing successful.\n";
    }

    ts_tree_delete(tree);
    ts_parser_delete(parser);

    return !hasErrors;
}