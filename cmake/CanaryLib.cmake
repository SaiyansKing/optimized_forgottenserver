# *****************************************************************************
# Cmake Features
# *****************************************************************************

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Global dá erro no spdlog
# Avoid changes by cmake/make on the source tree
# https://gitlab.kitware.com/cmake/cmake/issues/18403
# set(CMAKE_DISABLE_SOURCE_CHANGES ON)
# set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Make will print more details
set(CMAKE_VERBOSE_MAKEFILE OFF)


# If CMake version is less than 3.15, the if block will be true, and the policy
# will be set to the current CMake version. If CMake is 3.15 or higher, the if
# block will be false, but the new syntax in cmake_minimum_required will be
# respected and this will continue to work properly!
# WARNING: MSVC's CMake server mode originally had a bug in reading this format,
# so if you need to support non-command line Windows builds for older MSVC
# versions, you will want to do this instead:
if(${CMAKE_VERSION} VERSION_LESS 3.15)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
else()
  cmake_policy(VERSION 3.15)
endif()


# *****************************************************************************
# Configure 3rd party variables
# *****************************************************************************
set(OTBR_3RD_PARTY_PATH             ${CMAKE_SOURCE_DIR}/3rd)
set(OTBR_EXTERNAL_PATH              ${CMAKE_SOURCE_DIR}/external)
set(OTBR_EXTERNAL_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/build/external)


# *****************************************************************************
# Add Directories
# *****************************************************************************
add_subdirectory(3rd)
add_subdirectory(src)


# *****************************************************************************
# Includes
# *****************************************************************************
include(OtbrOptions)


# *****************************************************************************
# Find Packages
# *****************************************************************************

if (WIN32)
    # On Windows you must use cryptopp-static because the dynamic link library
    # is the FIPS DLL (which is something you should avoid).
    find_package(cryptopp CONFIG REQUIRED)
    set(CRYPTOPP_LIBRARIES "cryptopp-static")
else ()
  find_package(CryptoPP REQUIRED)
endif ()

find_package(PugiXML REQUIRED)
find_package(MySQL REQUIRED)
find_package(Threads REQUIRED)
find_package(Boost 1.53.0 COMPONENTS system filesystem iostreams date_time REQUIRED)

# Selects LuaJIT if user defines or auto-detected
if (DEFINED USE_LUAJIT AND NOT USE_LUAJIT)
    set(FORCE_LUAJIT ${USE_LUAJIT})
else ()
    find_package(LuaJIT)
    set(FORCE_LUAJIT ${LuaJIT_FOUND})
endif ()
option(USE_LUAJIT "Use LuaJIT" ${FORCE_LUAJIT})

if (FORCE_LUAJIT)
    if (APPLE)
        set(CMAKE_EXE_LINKER_FLAGS "-pagezero_size 10000 -image_base 100000000")
    endif ()
else ()
    find_package(Lua REQUIRED)
endif ()

# *****************************************************************************
# Compile flags and checks
# *****************************************************************************

# # set(CMAKE_CXX_FLAGS     "${CMAKE_CXX_FLAGS} -std=c++11 -lstdc++ -lpthread -ldl")

# # compile flags
# if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
#   # using Clang
#   # cmake -DCMAKE_BUILD_TYPE=Release ..
#   set(CMAKE_CXX_FLAGS_RELEASE   "-pipe -fvisibility=hidden -march=native -O3 -Wno-everything")

#   # cmake -DCMAKE_BUILD_TYPE=Debug ..
#   set(CMAKE_CXX_FLAGS_DEBUG     "-pipe -O0 -g -Wno-everything")

# elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
#   # using GCC

#   #Check GCC version
#   if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "5.1")
#     message(FATAL_ERROR "Insufficient gcc version. It has to be equal or greater than version 5.X")
#   endif()

#   # GCC is fairly aggressive about it: enabling strict aliasing will cause it to
#   # think that pointers that are "obviously" equivalent to a human (as in,
#   # foo *a; bar *b = (bar *) a;) cannot alias, which allows for some very
#   # aggressive transformations, but can obviously break non-carefully written
#   # code.
#   add_compile_options(-fno-strict-aliasing)

#   # https://kristerw.blogspot.com/2017/09/useful-gcc-warning-options-not-enabled.html

#   # cmake -DCMAKE_BUILD_TYPE=Release ..
#   # set(CMAKE_CXX_FLAGS_RELEASE   " -Wno-format -pipe -fvisibility=hidden -march=native -O3")

#   # cmake -DCMAKE_BUILD_TYPE=Debug ..
#   # Ubuntu 16.04
#   if(CMAKE_CXX_COMPILER_VERSION STREQUAL "5.4.0")
#     set(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -Werror -pipe -O0 -g -Wconversion -Wlogical-op -Wold-style-cast -Wuseless-cast -Wdouble-promotion -Wshadow -Wformat=2 -Wno-error")
#   else()
#     # Ubuntu 18.04
#     set(CMAKE_CXX_FLAGS_DEBUG   "-Wall -Wextra -Werror -pipe -O0 -g -Wconversion -Wduplicated-cond -Wduplicated-branches -Wlogical-op -Wrestrict -Wnull-dereference -Wold-style-cast -Wuseless-cast -Wdouble-promotion -Wshadow -Wformat=2 -Wno-error -w")
#   endif()

# elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
#   # using Visual Studio C++
#   # cmake -DCMAKE_BUILD_TYPE=Release ..
#   set(CMAKE_CXX_FLAGS_RELEASE   " /W4")

#   # cmake -DCMAKE_BUILD_TYPE=Debug ..
#   set(CMAKE_CXX_FLAGS_DEBUG     " /W4")
# endif()


# *****************************************************************************
# Library
# *****************************************************************************
add_library(otbr_lib ${otbr_SRC})


# *****************************************************************************
# Git Version
# *****************************************************************************

# Define the two required variables before including
# the source code for watching a git repository.
set(PRE_CONFIGURE_FILE "cmake/gitmetadata.h.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/gitmetadata.h")
include(GitWatcher)
if (Git_FOUND)
    message("-- Git Found")
    add_dependencies(otbr_lib check_git)
    target_include_directories(otbr_lib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()


# *****************************************************************************
# Includes & Libraries
# *****************************************************************************
include_directories(SYSTEM ${fmt_INCLUDE_DIRS} ${MYSQL_INCLUDE_DIR}
                    ${LUA_INCLUDE_DIR} ${Boost_INCLUDE_DIRS}
                    ${PUGIXML_INCLUDE_DIR} ${CRYPTOPP_INCLUDE_DIR})


# *****************************************************************************
# otbr_lib build options
# *****************************************************************************
target_link_libraries(otbr_lib PRIVATE spdlog::spdlog ${MYSQL_CLIENT_LIBS}
                      ${LUA_LIBRARIES} ${Boost_LIBRARIES}
                      ${Boost_FILESYSTEM_LIBRARY} ${PUGIXML_LIBRARIES}
                      ${CRYPTOPP_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})

# target_compile_definitions(otbr_lib PRIVATE -w)
