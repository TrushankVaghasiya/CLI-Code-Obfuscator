
cmake_minimum_required(VERSION 3.20)

project(cpp_obfuscator
    VERSION 0.1.0
    DESCRIPTION "C++ Source Code Obfuscator CLI"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Download CLI11 during CMake configuration
FetchContent_Declare(
    CLI11
    GIT_REPOSITORY https://github.com/CLIUtils/CLI11.git
    GIT_TAG v2.5.0
)

FetchContent_MakeAvailable(CLI11)

add_executable(obfuscator
    src/main.cpp
    src/cli.cpp
    src/validator.cpp
)

target_include_directories(obfuscator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(obfuscator
    PRIVATE
        CLI11::CLI11
)


include(CTest)

if(BUILD_TESTING)
    set(TEST_INPUT
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/data/sample.cpp"
    )

    set(TEST_OUTPUT
        "${CMAKE_CURRENT_BINARY_DIR}/test-output.cpp"
    )

    # Help command should succeed.
    add_test(
        NAME cli_help
        COMMAND obfuscator --help
    )

    # Valid arguments should pass validation.
    add_test(
        NAME cli_valid_arguments
        COMMAND obfuscator
            -i "${TEST_INPUT}"
            -o "${TEST_OUTPUT}"
            --level 2
    )

    # Invalid level should fail.
    add_test(
        NAME cli_invalid_level
        COMMAND obfuscator
            -i "${TEST_INPUT}"
            -o "${TEST_OUTPUT}"
            --level 5
    )
    set_tests_properties(
        cli_invalid_level
        PROPERTIES WILL_FAIL TRUE
    )

    # Missing input should fail validation.
    add_test(
        NAME cli_missing_input
        COMMAND obfuscator
            -i "${CMAKE_CURRENT_SOURCE_DIR}/missing.cpp"
            -o "${TEST_OUTPUT}"
            --level 2
    )
    set_tests_properties(
        cli_missing_input
        PROPERTIES WILL_FAIL TRUE
    )

    # Input and output must differ.
    add_test(
        NAME cli_same_input_output
        COMMAND obfuscator
            -i "${TEST_INPUT}"
            -o "${TEST_INPUT}"
            --level 2
    )
    set_tests_properties(
        cli_same_input_output
        PROPERTIES WILL_FAIL TRUE
    )
endif()

