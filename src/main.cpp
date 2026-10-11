
#include "cli.hpp"
#include "validator.hpp"
#include "parser.hpp"
#include "identifier_analyzer.hpp"
#include "transformer.hpp"
#include <CLI/CLI.hpp>
#include <iostream>
#include <string>

int main(int argc, char* argv[]) {
    CLI::App app{"C++ Source Code Obfuscator CLI"};

    app.set_version_flag("--version", "0.1.0");

    CliOptions options;
    configureCLI(app, options);

    try {
        app.parse(argc, argv);
    } catch (const CLI::ParseError& e) {
        return app.exit(e);
    }

    std::string error;

    if (!validateOptions(options, error)) {
        std::cerr << "[ERROR] " << error << '\n';
        return 2;
    }

    std::cout << "[INFO] CLI arguments parsed successfully.\n";
    std::cout << "[INFO] Input:  " << options.inputPath << '\n';
    std::cout << "[INFO] Output: " << options.outputPath << '\n';
    std::cout << "[INFO] Level:  " << options.level << '\n';
    std::cout << "[INFO] Validation successful.\n";

    if (!parseCppFile(options.inputPath, error)) {
        std::cerr << "[ERROR] " << error << '\n';
        return 3;
    }

    std::cout << "[INFO] Phase 3 parsing completed successfully.\n";

    if (!analyzeIdentifiers(options.inputPath, error)) {
        std::cerr << "[ERROR] " << error << '\n';
        return 4;
    }

    std::cout << "[INFO] Identifier analysis completed.\n"; 

    if (!transformCppFile(
        options.inputPath,
        options.outputPath,
        options.level,
        error)) { {
        std::cerr << "[ERROR] " << error << '\n';
        return 5;
    }

    std::cout << "[INFO] Transformation completed successfully.\n";

    return 0;
}
}
