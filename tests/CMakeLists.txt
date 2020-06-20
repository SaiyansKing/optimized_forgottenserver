cmake_minimum_required(VERSION 3.0)

project(canary_tests)
set(CMAKE_CXX_FLAGS "-pipe -O0 -g -Wno-everything -std=c++11 -lstdc++ -lpthread -ldl")

include_directories(${MYSQL_INCLUDE_DIR} ${LUA_INCLUDE_DIR} ${Boost_INCLUDE_DIRS} ${PUGIXML_INCLUDE_DIR} ${GMP_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})

# Prepare "Catch" library for other executables
set(CATCH_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/catch)
add_library(Catch INTERFACE)
target_include_directories(Catch INTERFACE ${CATCH_INCLUDE_DIR})

add_subdirectory(src)

# Make test executable
add_executable(canary_tests ${test_SRC})

target_compile_definitions(canary_tests PUBLIC -DUNIT_TESTING -DDEBUG_LOG)

target_link_libraries(canary_tests Catch canary_lib ${MYSQL_CLIENT_LIBS}  
  ${LUA_LIBRARIES} ${Boost_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY} ${PUGIXML_LIBRARIES} 
  ${GMP_LIBRARIES} ${ZLIB_LIBRARY} ${CMAKE_THREAD_LIBS_INIT})