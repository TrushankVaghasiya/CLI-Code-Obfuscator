
#pragma once

#include <string>

bool transformCppFile(
    const std::string& inputPath,
    const std::string& outputPath,
    int level,
    std::string& error
);
