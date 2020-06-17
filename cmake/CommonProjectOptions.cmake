# === BUILD TYPE ===
# -DCMAKE_BUILD_TYPE=<Release|Debug|RelWithDebInfo>
set_property(CACHE CMAKE_BUILD_TYPE
             PROPERTY STRINGS "Debug"
                              "Release"
                              "RelWithDebInfo"
            )
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    log_war("No build type specified, setting to RelWithDebInfo")
    set(CMAKE_BUILD_TYPE
        RelWithDebInfo
        CACHE STRING "Type of build" FORCE
       )
else()
    log_info("Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(BUILD_SHARED_LIBS OFF)

# === CCACHE ===
option(OPTIONS_ENABLE_CCACHE
       "Enable ccache"
       ON
      )
if(OPTIONS_ENABLE_CCACHE)
    find_program(CCACHE ccache)
    if(CCACHE)
        log_option_enabled("ccache")
        set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE})
    else()
        log_option_disabled("ccache")
    endif()
endif()

# === LTO ===
option(OPTIONS_ENABLE_LTO
       "Enable link-time optimization"
       ON)
if(OPTIONS_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT result OUTPUT output)
    if(result)
        set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    else()
        log_err("LTO is not supported: ${output}")
    endif()
endif()

# === PCH ===
# target_precompile_headers(common_project_options INTERFACE <memory>
#                                                            <set>
#                                                            <string>
#                                                            <vector>
#                                                            <cassert>
#                                                            <cstdint>
#                          )

# target_compile_options(common_project_options INTERFACE -std=c++2a)
