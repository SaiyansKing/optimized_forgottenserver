cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

enable_testing()

project(canary-tests)

add_subdirectory(src)

# Make test executable
add_executable(canary-tests main.cpp ${CANARY_TEST_SRC})

target_compile_definitions(canary-tests PUBLIC -DUNIT_TESTING -DDEBUG_LOG)

target_link_libraries(canary-tests doctest::doctest canary_lib)

set_target_properties(canary-tests
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)

doctest_discover_tests(canary-tests
                      WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/src)
