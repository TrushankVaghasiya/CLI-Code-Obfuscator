
if(NOT DEFINED OBFUSCATOR OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "Missing test arguments")
endif()

execute_process(
    COMMAND "${OBFUSCATOR}" -i "${INPUT}" -o "${OUTPUT}" --level 1
    RESULT_VARIABLE result
    OUTPUT_VARIABLE stdout
    ERROR_VARIABLE stderr
)

if(NOT result EQUAL 0)
    message(FATAL_ERROR
        "Obfuscator failed on empty input:\n${stdout}\n${stderr}")
endif()

if(NOT EXISTS "${OUTPUT}")
    message(FATAL_ERROR "Output file was not created")
endif()

file(SIZE "${OUTPUT}" output_size)

if(NOT output_size EQUAL 0)
    message(FATAL_ERROR
        "Expected empty output, but got ${output_size} bytes")
endif()

message(STATUS "Empty-input test passed")
