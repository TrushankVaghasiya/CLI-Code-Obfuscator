
#include "validator.hpp"

#include <algorithm>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace {

bool isSupportedExtension(const fs::path& path) {
    std::string extension = path.extension().string();

    std::transform(
        extension.begin(),
        extension.end(),
        extension.begin(),
        [](unsigned char c) {
            return static_cast<char>(std::tolower(c));
        }
    );

    return extension == ".cpp"
        || extension == ".cc"
        || extension == ".cxx";
}

} // namespace

bool validateOptions(
    const CliOptions& options,
    std::string& error
) {
    std::error_code ec;

    // 1. Check the input path.
    fs::path inputPath = fs::absolute(options.inputPath, ec);

    if (ec || !fs::exists(inputPath, ec) || ec) {
        error = "Input file does not exist: " + options.inputPath;
        return false;
    }

    if (!fs::is_regular_file(inputPath, ec) || ec) {
        error = "Input path is not a regular file.";
        return false;
    }

    if (!isSupportedExtension(inputPath)) {
        error = "Unsupported input extension. Use .cpp, .cc, or .cxx.";
        return false;
    }

    std::ifstream inputFile(inputPath, std::ios::binary);

    if (!inputFile) {
        error = "Cannot read input file: " + options.inputPath;
        return false;
    }

    // 2. Check the output path.
    fs::path outputPath = fs::absolute(options.outputPath, ec);

    if (ec) {
        error = "Cannot resolve the output path.";
        return false;
    }

    if (inputPath.lexically_normal() ==
        outputPath.lexically_normal()) {
        error = "Input and output paths must be different.";
        return false;
    }

    if (!isSupportedExtension(outputPath)) {
        error = "Output must use .cpp, .cc, or .cxx.";
        return false;
    }

    fs::path parent = outputPath.parent_path();

    if (!fs::exists(parent, ec) || ec ||
        !fs::is_directory(parent, ec) || ec) {
        error = "Output directory does not exist.";
        return false;
    }

    if (fs::exists(outputPath, ec)) {
        if (ec || !fs::is_regular_file(outputPath, ec) || ec) {
            error = "Output path is not a regular file.";
            return false;
        }

        std::ofstream existingOutput(
            outputPath,
            std::ios::binary | std::ios::app
        );

        if (!existingOutput) {
            error = "Cannot write to existing output file.";
            return false;
        }
    } else {
        // Test directory writability without creating the final output.
        auto stamp =
            std::chrono::steady_clock::now()
                .time_since_epoch()
                .count();

        fs::path probe = parent /
            (".obfuscator_write_test_" +
             std::to_string(stamp) + ".tmp");

        {
            std::ofstream testFile(probe, std::ios::binary);

            if (!testFile) {
                error = "Cannot write to the output directory.";
                return false;
            }

            testFile << "test";
            testFile.flush();

            if (!testFile) {
                testFile.close();
                fs::remove(probe, ec);
                error = "Failed while testing output permissions.";
                return false;
            }
        }

        fs::remove(probe, ec);

        if (ec) {
            error = "Could not remove temporary validation file: "
                  + probe.string();
            return false;
        }
    }

    return true;
}
