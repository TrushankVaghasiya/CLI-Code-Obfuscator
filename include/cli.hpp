
#pragma once

#include <CLI/CLI.hpp>
#include <string>

struct CliOptions {
    std::string inputPath;
    std::string outputPath;
    int level = 1;
};

void configureCLI(CLI::App& app, CliOptions& options);
