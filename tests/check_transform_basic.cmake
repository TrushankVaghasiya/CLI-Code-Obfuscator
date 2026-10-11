if(NOT DEFINED OBFUSCATOR OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "OBFUSCATOR, INPUT, and OUTPUT are required")
endif()

execute_process(
    COMMAND "${OBFUSCATOR}"
        -i "${INPUT}"
        -o "${OUTPUT}"
        --level 1
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Obfuscator failed.\nSTDOUT:\n${stdout}\nSTDERR:\n${stderr}")
endif()

if(NOT EXISTS "${OUTPUT}")
    message(FATAL_ERROR "Transformed output file was not created")
endif()

file(READ "${OUTPUT}" transformed)

if(NOT transformed MATCHES "int obf_var_1 = 5;")
    message(FATAL_ERROR "Expected variable declaration was not renamed")
endif()

if(transformed MATCHES "\\bnumber\\b")
    message(FATAL_ERROR "Original variable name remains in the output")
endif()

string(REGEX MATCHALL "obf_var_1" occurrences "${transformed}")
list(LENGTH occurrences occurrence_count)

string(REGEX MATCHALL "obf_var_1" occurrences "${transformed}")
list(LENGTH occurrences occurrence_count)

if(NOT occurrence_count EQUAL 4)
    message(FATAL_ERROR
        "Expected 4 occurrences of obf_var_1, found ${occurrence_count}")
endif()

message(STATUS "Basic identifier transformation test passed")