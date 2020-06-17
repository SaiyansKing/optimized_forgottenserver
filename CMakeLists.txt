cmake_minimum_required(VERSION 3.10 FATAL_ERROR)


# *****************************************************************************
# Project Canary
# *****************************************************************************
project(canary VERSION 1.0.0 LANGUAGES CXX)

# *****************************************************************************
# Configure 3rd party variables
# *****************************************************************************
set(CANARY_3RD_PARTY_PATH             ${CMAKE_SOURCE_DIR}/3rd)
set(CANARY_EXTERNAL_PATH              ${CMAKE_SOURCE_DIR}/external)
set(CANARY_EXTERNAL_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/build/external)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

include(MessageColors)
include(LoggingHelper)
include(TargetLinkLibrariesSystem)

add_library(common_project_options INTERFACE)

include(CommonProjectOptions)

include(ToolCppCheck)
include(ToolClangTidy)
include(ToolIWYU)
include(ToolClangFormat)
include(ToolCTags)
include(ToolGitSubmodule)

include(OptionCompilerWarnings)
# include(OtbrLib)
# include(Otbr)
add_subdirectory(3rd)
add_subdirectory(src)
