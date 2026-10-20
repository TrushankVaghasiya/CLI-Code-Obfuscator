
cmake_minimum_required(VERSION 3.20)

project(cpp_obfuscator
    VERSION 0.1.0
    DESCRIPTION "C++ Source Code Obfuscator CLI"
    LANGUAGES C CXX
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
    src/parser.cpp
    src/identifier_analyzer.cpp
    src/transformer.cpp
)

target_include_directories(obfuscator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(obfuscator
    PRIVATE
        CLI11::CLI11
        tree_sitter_runtime_lib
        tree_sitter_cpp_grammar
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

    add_test(
    NAME transform_basic
    COMMAND ${CMAKE_COMMAND}
        "-DOBFUSCATOR=$<TARGET_FILE:obfuscator>"
        "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/tests/data/transform_basic.cpp"
        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/test-transform-basic.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_transform_basic.cmake"
    )

    
add_test(
    NAME level1_preserves_comments
    COMMAND ${CMAKE_COMMAND}
        "-DOBFUSCATOR=$<TARGET_FILE:obfuscator>"
        "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/tests/data/comments_strings.cpp"
        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/test-level1.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_level1_comments.cmake"
)


add_test(
    NAME transform_shadowing
    COMMAND ${CMAKE_COMMAND}
        "-DOBFUSCATOR=$<TARGET_FILE:obfuscator>"
        "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/tests/data/shadowing.cpp"
        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/shadowing-test.cpp"
        "-DCOMPILER=g++"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_shadowing.cmake"
)


add_test(
    NAME transform_empty_input
    COMMAND ${CMAKE_COMMAND}
        "-DOBFUSCATOR=$<TARGET_FILE:obfuscator>"
        "-DINPUT=${CMAKE_CURRENT_SOURCE_DIR}/tests/data/empty.cpp"
        "-DOUTPUT=${CMAKE_CURRENT_BINARY_DIR}/empty-output.cpp"
        -P "${CMAKE_CURRENT_SOURCE_DIR}/tests/check_empty_input.cmake"
)



endif()

# Tree-sitter runtime and C++ grammar
FetchContent_Declare(
    tree_sitter_runtime
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter.git
    GIT_TAG v0.25.10
    GIT_SHALLOW TRUE
)

FetchContent_Declare(
    tree_sitter_cpp
    GIT_REPOSITORY https://github.com/tree-sitter/tree-sitter-cpp.git
    GIT_TAG v0.23.4
    GIT_SHALLOW TRUE
)

# Download the source repositories without adding their own
# projects to our build.
FetchContent_GetProperties(tree_sitter_runtime)
if(NOT tree_sitter_runtime_POPULATED)
    FetchContent_Populate(tree_sitter_runtime)
endif()

FetchContent_GetProperties(tree_sitter_cpp)
if(NOT tree_sitter_cpp_POPULATED)
    FetchContent_Populate(tree_sitter_cpp)
endif()

# Build the Tree-sitter runtime.
add_library(tree_sitter_runtime_lib STATIC
    "${tree_sitter_runtime_SOURCE_DIR}/lib/src/lib.c"
)

target_include_directories(tree_sitter_runtime_lib
    PUBLIC
        "${tree_sitter_runtime_SOURCE_DIR}/lib/include"
    PRIVATE
        "${tree_sitter_runtime_SOURCE_DIR}/lib/src"
)

set_target_properties(tree_sitter_runtime_lib PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

# Build the generated C++ grammar and its external scanner.
add_library(tree_sitter_cpp_grammar STATIC
    "${tree_sitter_cpp_SOURCE_DIR}/src/parser.c"
    "${tree_sitter_cpp_SOURCE_DIR}/src/scanner.c"
)

target_include_directories(tree_sitter_cpp_grammar
    PRIVATE
        "${tree_sitter_runtime_SOURCE_DIR}/lib/include"
        "${tree_sitter_cpp_SOURCE_DIR}/src"
)

set_target_properties(tree_sitter_cpp_grammar PROPERTIES
    C_STANDARD 11
    C_STANDARD_REQUIRED ON
)

