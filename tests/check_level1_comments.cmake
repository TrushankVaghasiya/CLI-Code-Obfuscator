
if(NOT DEFINED OBFUSCATOR OR NOT DEFINED INPUT OR NOT DEFINED OUTPUT)
    message(FATAL_ERROR "Missing test arguments")
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
        "Obfuscator failed:\n${stdout}\n${stderr}")
endif()

file(READ "${OUTPUT}" transformed)

if(NOT transformed MATCHES "This is a comment")
    message(FATAL_ERROR "Level 1 unexpectedly removed the comment")
endif()

if(NOT transformed MATCHES "text containing // comment markers")
    message(FATAL_ERROR "String literal was changed")
endif()

message(STATUS "Level 1 comment-preservation test passed")
