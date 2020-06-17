project(otbr_unittest)

add_executable(otbr_unittest
              main.cpp
              account_test.cpp)

target_compile_definitions(otbr_unittest PUBLIC UNIT_TESTING=1 DEBUG_LOG=1 -fvar-tracking-assignments)

target_link_libraries(otbr_unittest Catch2::Catch2 otbr_lib)

include(CTest)
include(Catch)
catch_discover_tests(otbr_unittest)
