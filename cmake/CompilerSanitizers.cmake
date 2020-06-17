if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
   CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(SANITIZERS "")

    option(SANITIZERS_ENABLE_ADDRESS
           "Enable address sanitizer"
           FALSE
          )
    if(SANITIZERS_ENABLE_ADDRESS)
        log_option_enabled("address sanitizer")
        list(APPEND SANITIZERS "address")
    else()
        log_option_disabled("address_sanitizer")
    endif()

    option(SANITIZERS_ENABLE_MEMORY
           "Enable memory sanitizer"
           FALSE
          )
    if(SANITIZERS_ENABLE_MEMORY)
        log_option_enabled("memory sanitizer")
        list(APPEND SANITIZERS "memory")
    else()
        log_option_disabled("memory sanitizer")
    endif()

    option(SANITIZERS_ENABLE_UB
           "Enable undefined behavior sanitizer"
           FALSE)
    if(SANITIZERS_ENABLE_UB)
        log_option_enabled("undefined behavior sanitizer")
        list(APPEND SANITIZERS "undefined")
    else()
        log_option_disabled("undefined behavior sanitizer")
    endif()

    list(JOIN SANITIZERS "," SANITIZERS_LIST)

    if(SANITIZERS_LIST)
        if(NOT "${SANITIZERS_LIST}" STREQUAL "")
            target_compile_options(common_project_options
                                   INTERFACE -fsanitize=${SANITIZERS_LIST}
                                  )
            target_link_libraries(common_project_options
                                  INTERFACE -fsanitize=${SANITIZERS_LIST}
                                 )
        endif()
    endif()
endif()
