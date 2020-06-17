function(target_link_libraries_system TARGET)
    math(EXPR _ARGC ${ARGC}-1)

    foreach(IDX RANGE 1 ${_ARGC} 2)
        math(EXPR _IDX ${IDX}+1)
        set(LIB ${ARGV${_IDX}})

        get_target_property(LIB_INCLUDE_DIRS ${LIB} INTERFACE_INCLUDE_DIRECTORIES)
        target_include_directories(${TARGET} SYSTEM ${ARGV${IDX}} ${LIB_INCLUDE_DIRS})
        target_link_libraries(${TARGET} ${ARGV${IDX}} ${LIB})
    endforeach()
endfunction()
