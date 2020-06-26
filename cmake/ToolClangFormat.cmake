option(TOOLS_ENABLE_FORMATTING
       "Enable automatic code formatting with clang-format"
       OFF)

if(TOOLS_ENABLE_FORMATTING)
    find_program(CLANG-FORMAT clang-format)
    if(CLANG-FORMAT)
        log_option_enabled("formatting")
    else()
        log_program_missing("clang-format")
    endif()
else()
    log_option_disabled("formatting")
endif()


function(add_format_target TARGET)
    if(TOOLS_ENABLE_FORMATTING)
        get_target_property(TARGET_DIR ${TARGET} SOURCE_DIR)
        get_target_property(TARGET_SOURCES ${TARGET} SOURCES)

        foreach(FILE ${TARGET_SOURCES})
            get_filename_component(EXTENSION ${FILE} EXT)
            if(NOT "${EXTENSION}" MATCHES ".spv$")
                get_filename_component(FILE ${FILE} REALPATH BASE_DIR ${TARGET_DIR})
                list(APPEND FORMAT_SOURCES ${FILE})
            endif()
        endforeach()

        file(GLOB_RECURSE GLOBAL_HEADERS ${CMAKE_SOURCE_DIR}/include/*.hh)
        foreach(FILE ${GLOBAL_HEADERS})
            list(APPEND FORMAT_SOURCES ${FILE})
        endforeach()

        set(FORMAT_TARGET_NAME ${TARGET}_format)

        add_custom_target(${FORMAT_TARGET_NAME}
                          COMMAND ${CLANG-FORMAT} -i -style=Google ${FORMAT_SOURCES}
                          COMMENT "Formatting ${TARGET}..."
                         )

        add_dependencies(${TARGET} ${FORMAT_TARGET_NAME})
    endif()
endfunction()
