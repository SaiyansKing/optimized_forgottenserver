set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# *****************************************************************************
# Executable
# *****************************************************************************
add_executable(otbr ${CMAKE_CURRENT_SOURCE_DIR}/src/otserv.cpp)


# *****************************************************************************
# otbr build options
# *****************************************************************************
target_link_libraries(otbr PRIVATE otbr_lib)
target_compile_definitions(otbr INTERFACE UNIT_TESTING=1)
