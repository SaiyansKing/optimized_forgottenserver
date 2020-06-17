# === Debug Log ===
# -DOPTIONS_ENABLE_DEBUG_LOG=1
option(OPTIONS_ENABLE_DEBUG_LOG
       "Enable ccache"
       OFF
      )
if(OPTIONS_ENABLE_DEBUG_LOG)
  add_definitions( -DDEBUG_LOG )
  log_option_enabled("debug log")
else()
  log_option_disabled("debug log")
endif()

# === Source Code Doxygen Documentation ===
# -DOPTIONS_ENABLE_DOXYGEN=1
option(OPTIONS_ENABLE_DOXYGEN
      "Build source code documentation"
      OFF)
if(OPTIONS_ENABLE_DOXYGEN)

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)

  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    log_option_enabled("doxygen")

    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    # request to configure the file
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    #make doc_doxygen
    add_custom_target( doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM )

  else ()
    log_program_missing("Doxygen need to be installed to generate the doxygen documentation")
  endif ()

else()
  log_option_disabled("doxygen")
endif()


# === Datapack  LDoc Documentation ===
# -DOPTIONS_ENABLE_LDOC=1
option(OPTIONS_ENABLE_LDOC
      "Build datapack documentation"
      OFF)
if(OPTIONS_ENABLE_LDOC)

  find_program(LDOC_EXECUTABLE ldoc FALSE)
  if(NOT LDOC_EXECUTABLE)
    find_program(LDOC_EXECUTABLE ldoc.lua FALSE)
  endif()

  if (LDOC_EXECUTABLE)
    log_option_enabled("ldoc")

    # set input and output files
    set(LDOC_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/config.ld.in)
    set(LDOC_OUT ${CMAKE_CURRENT_BINARY_DIR}/config.ld)

    # request to configure the file
    configure_file(${LDOC_IN} ${LDOC_OUT} @ONLY)
    # execute_process(COMMAND sh -c "${LDOC_EXECUTABLE} .")

    #make doc_ldoc
    add_custom_target( doc_ldoc ALL
        COMMAND ${LDOC_EXECUTABLE} .
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Ldoc")
  else ()
    log_program_missing("Ldoc need to be installed to generate the ldoc documentation")
  endif ()

else()
  log_option_disabled("ldoc")
endif()

# === Unit Test ===
# -DOPTIONS_ENABLE_UNIT_TEST=1
option(OPTIONS_ENABLE_UNIT_TEST
       "Enable Unit Test Build"
       OFF
      )
if(OPTIONS_ENABLE_UNIT_TEST)
  enable_testing()
  add_subdirectory(tests)
  target_compile_definitions(common_project_options INTERFACE UNIT_TESTING=1)
else()
  log_option_disabled("unit test")
endif()
