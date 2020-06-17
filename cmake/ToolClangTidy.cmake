option(TOOLS_ENABLE_TIDY
       "Enable static analysis with clang-tidy"
       OFF)

if(TOOLS_ENABLE_TIDY)
    find_program(CLANG-TIDY clang-tidy)
    if(CLANG-TIDY)
        log_option_enabled("clang-tidy")
        set(CMAKE_CXX_CLANG_TIDY ${CLANG-TIDY})
    else()
        log_program_missing("clang-tidy")
    endif()
else()
    log_option_disabled("clang-tidy")
endif()
