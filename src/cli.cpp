
#include "cli.hpp"

void configureCLI(CLI::App& app, CliOptions& options) {
    app.description("C++ Source Code Obfuscator CLI");

    app.add_option(
        "-i,--input",
        options.inputPath,
        "Input C++ source file"
    )->required();

    app.add_option(
        "-o,--output",
        options.outputPath,
        "Output file for transformed source"
    )->required();

    app.add_option(
        "--level",
        options.level,
        "Obfuscation level (1-3)"
    )
        ->default_val(1)
        ->check(CLI::Range(1, 3));
}
