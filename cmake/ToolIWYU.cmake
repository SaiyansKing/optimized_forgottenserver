option(TOOLS_ENABLE_IWYU
       "Enable static analysis with include-what-you-use"
       FALSE)

if(TOOLS_ENABLE_IWYU)
    find_program(IWYU NAMES include-what-you-use iwyu)
    if(IWYU)
        log_option_enabled("iwyu")
        set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE ${IWYU})
    else()
        log_program_missing("iwyu")
    endif()
else()
    log_option_disabled("iwyu")
endif()
