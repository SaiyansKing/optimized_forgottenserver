cmake_minimum_required(VERSION 3.10 FATAL_ERROR)


# *****************************************************************************
# Project Canary
# *****************************************************************************
project(canary VERSION 1.0.0 LANGUAGES CXX)


# *****************************************************************************
# Options
# *****************************************************************************
option(OPTIONS_ENABLE_CCACHE "Enable ccache" ON)
option(OPTIONS_ENABLE_IPO "Check and Enable interprocedural optimization (IPO/LTO)" ON)


# *****************************************************************************
# Configure CanaryLib variable
# *****************************************************************************
set(CANARY_LIB_PATH ${CMAKE_SOURCE_DIR}/canary-lib)


# *****************************************************************************
# Configure 3rd party variables
# *****************************************************************************
set(CANARY_3RD_PARTY_PATH             ${CMAKE_SOURCE_DIR}/3rd)
set(CANARY_EXTERNAL_PATH              ${CMAKE_SOURCE_DIR}/external)
set(CANARY_EXTERNAL_OUTPUT_DIRECTORY  ${CMAKE_SOURCE_DIR}/build/external)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)


# *****************************************************************************
# Include cmake tools
# *****************************************************************************
include(MessageColors)
include(LoggingHelper)
include(ToolGitSubmodule)


# *****************************************************************************
# Options Code
# *****************************************************************************

# === CCACHE ===
if(OPTIONS_ENABLE_CCACHE)
    find_program(CCACHE ccache)
    if(CCACHE)
        log_option_enabled("ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
        else()
        log_option_disabled("ccache")
    endif()
endif()


# === IPO ===
if(OPTIONS_ENABLE_IPO)
  log_option_enabled("ipo")

  include(CheckIPOSupported)
  check_ipo_supported(RESULT result OUTPUT output)
  if(result)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
  else()
    log_war("IPO is not supported: ${output}")
  endif()
else()
  log_option_disabled("ipo")
endif()


# *****************************************************************************
# Add 3rd party libraries
# *****************************************************************************
add_subdirectory(3rd)
add_subdirectory(canary-lib)


# *****************************************************************************
# Add Source Code
# *****************************************************************************
add_subdirectory(src)
