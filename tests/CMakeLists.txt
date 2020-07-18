cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

project(canary-tests)
add_subdirectory(src)

# Make test executable
add_executable(canary-tests main.cpp ${CANARY_TEST_SRC})
target_compile_definitions(canary-tests PUBLIC -DUNIT_TESTING -DDEBUG_LOG)
target_link_libraries(canary-tests canary_lib::server_lib)
set_target_properties(canary-tests
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
