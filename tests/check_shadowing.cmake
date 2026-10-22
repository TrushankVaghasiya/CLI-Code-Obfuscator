
if(NOT DEFINED OBFUSCATOR OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT OR NOT DEFINED COMPILER)
    message(FATAL_ERROR "Missing test arguments")
endif()

execute_process(
    COMMAND "${OBFUSCATOR}" -i "${INPUT}" -o "${OUTPUT}" --level 1
    RESULT_VARIABLE transform_result
    OUTPUT_VARIABLE transform_stdout
    ERROR_VARIABLE transform_stderr
)

if(NOT transform_result EQUAL 0)
    message(FATAL_ERROR "Transformation failed:\n${transform_stdout}\n${transform_stderr}")
endif()

execute_process(
    COMMAND "${COMPILER}" "${OUTPUT}" -o "${OUTPUT}.exe"
    RESULT_VARIABLE compile_result
    OUTPUT_VARIABLE compile_stdout
    ERROR_VARIABLE compile_stderr
)

if(NOT compile_result EQUAL 0)
    message(FATAL_ERROR "Compilation failed:\n${compile_stdout}\n${compile_stderr}")
endif()

execute_process(
    COMMAND "${OUTPUT}.exe"
    RESULT_VARIABLE run_result
    OUTPUT_VARIABLE actual_output
    ERROR_VARIABLE run_stderr
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

if(NOT run_result EQUAL 0)
    message(FATAL_ERROR "Transformed program failed to run:\n${run_stderr}")
endif()

string(REPLACE "\r\n" "\n" actual_output "${actual_output}")

if(NOT actual_output STREQUAL "20\n10")
    message(FATAL_ERROR "Unexpected output.\nExpected:\n20\n10\nActual:\n${actual_output}\n${run_stderr}")
endif()

message(STATUS "Shadowing behavior test passed")
