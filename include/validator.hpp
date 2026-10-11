
#pragma once

#include "cli.hpp"
#include <string>

bool validateOptions(
    const CliOptions& options,
    std::string& error
);
