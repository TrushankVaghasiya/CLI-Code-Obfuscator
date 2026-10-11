
#include "cli.hpp"
#include "validator.hpp"

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
    std::cout << "[INFO] Obfuscation pipeline is not implemented yet.\n";

    return 0;
}
