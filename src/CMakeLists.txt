# *****************************************************************************
# Cmake Features
# *****************************************************************************
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Global gives error when building spdlog
# Avoid changes by cmake/make on the source tree
# https://gitlab.kitware.com/cmake/cmake/issues/18403
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Make will print more details
set(CMAKE_VERBOSE_MAKEFILE OFF)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)


# *****************************************************************************
# Options
# *****************************************************************************
option(OPTIONS_ENABLE_CTAGS "Enable automatic tags generation with ctags" OFF)
option(OPTIONS_ENABLE_DEBUG_LOG "Enable debug log" OFF)
option(OPTIONS_ENABLE_DOXYGEN "Build source code documentation" OFF)
option(OPTIONS_ENABLE_LDOC "Build datapack documentation" OFF)
option(OPTIONS_ENABLE_OPENMP "Enable Open Multi-Processing support." ON)
option(OPTIONS_ENABLE_UNIT_TEST "Enable Unit-Test Build" OFF)
option(OPTIONS_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(OPTIONS_WARNINGS_FLAGS "Enable the warning flags" OFF)


# *****************************************************************************
# Find Packages
# *****************************************************************************
find_package(Threads REQUIRED)
find_package(Boost 1.53.0 COMPONENTS system filesystem iostreams date_time REQUIRED)
find_package(Lua REQUIRED)
find_package(ZLIB REQUIRED)

# *****************************************************************************
# Create Library
# *****************************************************************************
add_library(canary_server_lib STATIC "")


# *****************************************************************************
# Define Default Build Type
# *****************************************************************************
# -DCMAKE_BUILD_TYPE=<Release|Debug|RelWithDebInfo>
set(default_build_type "RelWithDebInfo")
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  log_info("Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()


# *****************************************************************************
# Options Code
# *****************************************************************************

# === CTAGS ===
if(OPTIONS_ENABLE_CTAGS)
  log_option_enabled("tags")
  find_program(CTAGS ctags)
  if(CTAGS)
      add_custom_target(tags_generation ALL
                        COMMAND ${CTAGS} -R --fields=+iaS --extra=+q --language-force=C++ ${CMAKE_CURRENT_SOURCE_DIR}
                        COMMENT "Generating tags file..."
                        )
  else()
      log_program_missing("ctags")
  endif()
else()
    log_option_disabled("tags")
endif()

# === OpenMP ===
if(OPTIONS_ENABLE_OPENMP)
  log_option_enabled("openmp")
  find_package(OpenMP)
  if(OpenMP_CXX_FOUND)
      target_link_libraries(canary_server_lib PUBLIC OpenMP::OpenMP_CXX)
  endif()
else()
  log_option_disabled("openmp")
endif()


# === Add Warnings Flags ===
if(OPTIONS_WARNINGS_FLAGS)
  log_option_enabled("warning flags")
  set(CLANG_WARNINGS -Wall
                    -Wextra
                    -Wpedantic
                    -Wshadow
                    -Wnon-virtual-dtor
                    -Wold-style-cast
                    -Wcast-align
                    -Wunused
                    -Woverloaded-virtual
                    -Wsign-conversion
                    -Wnull-dereference
                    -Wdouble-promotion
                    -Wformat=2
                    -Wno-c99-extensions
    )

  set(GCC_WARNINGS ${CLANG_WARNINGS}
                  -Wmisleading-indentation
                  -Wduplicated-cond
                  -Wduplicated-branches
                  -Wlogical-op
                  -Wuseless-cast
    )

  if(MSVC)
    set(PROJECT_WARNINGS_FLAGS -w)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(PROJECT_WARNINGS_FLAGS ${CLANG_WARNINGS})
  else()
    set(PROJECT_WARNINGS_FLAGS ${GCC_WARNINGS})
  endif()
else()
  log_option_disabled("warning flags")
endif()


# === Set Warnings as Error Flag ===
if(OPTIONS_WARNINGS_AS_ERRORS)
  log_option_enabled("warning as error")
  set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)

  if(MSVC)
    set(PROJECT_WARNINGS_FLAGS -w)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(PROJECT_WARNINGS_FLAGS ${CLANG_WARNINGS})
  else()
    set(PROJECT_WARNINGS_FLAGS ${GCC_WARNINGS})
  endif()
else()
  log_option_disabled("warning as error")
endif()


# === Debug Log ===
# -DOPTIONS_ENABLE_DEBUG_LOG=ON
if(OPTIONS_ENABLE_DEBUG_LOG)
  add_definitions( -DDEBUG_LOG )
  log_option_enabled("debug log")
else()
  log_option_disabled("debug log")
endif()


# === Source Code Doxygen Documentation ===
# -DOPTIONS_ENABLE_DOXYGEN=ON
if(OPTIONS_ENABLE_DOXYGEN)

  file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/doc)

  find_package(Doxygen)
  if (DOXYGEN_FOUND)
    log_option_enabled("doxygen")

    # set input and output files
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
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
# -DOPTIONS_ENABLE_LDOC=ON
if(OPTIONS_ENABLE_LDOC)

  find_program(LDOC_EXECUTABLE ldoc FALSE)
  if(NOT LDOC_EXECUTABLE)
    find_program(LDOC_EXECUTABLE ldoc.lua FALSE)
  endif()

  if (LDOC_EXECUTABLE)
    log_option_enabled("ldoc")

    # set input and output files
    set(LDOC_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/config.ld.in)
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
# -DOPTIONS_ENABLE_UNIT_TEST=ON
if(OPTIONS_ENABLE_UNIT_TEST)
  log_option_enabled("unit-test")
  add_subdirectory(../tests tests)
  target_compile_definitions(canary_server_lib PRIVATE UNIT_TESTING=1)
else()
  log_option_disabled("unit-test")
endif()


# *****************************************************************************
# Library Configuration
# *****************************************************************************
target_sources(canary_server_lib
  PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/actions.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ban.cpp
    ${CMAKE_CURRENT_LIST_DIR}/baseevents.cpp
    ${CMAKE_CURRENT_LIST_DIR}/bed.cpp
    ${CMAKE_CURRENT_LIST_DIR}/chat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/combat.cpp
    ${CMAKE_CURRENT_LIST_DIR}/condition.cpp
    ${CMAKE_CURRENT_LIST_DIR}/configmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/connection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/container.cpp
    ${CMAKE_CURRENT_LIST_DIR}/creature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/creatureevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cylinder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/database.cpp
    ${CMAKE_CURRENT_LIST_DIR}/databasemanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/databasetasks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/decay.cpp
    ${CMAKE_CURRENT_LIST_DIR}/depotchest.cpp
    ${CMAKE_CURRENT_LIST_DIR}/depotlocker.cpp
    ${CMAKE_CURRENT_LIST_DIR}/events.cpp
    ${CMAKE_CURRENT_LIST_DIR}/fileloader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/game.cpp
    ${CMAKE_CURRENT_LIST_DIR}/globalevent.cpp
    ${CMAKE_CURRENT_LIST_DIR}/guild.cpp
    ${CMAKE_CURRENT_LIST_DIR}/groups.cpp
    ${CMAKE_CURRENT_LIST_DIR}/house.cpp
    ${CMAKE_CURRENT_LIST_DIR}/housetile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/inbox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ioguild.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iologindata.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iomap.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iomapserialize.cpp
    ${CMAKE_CURRENT_LIST_DIR}/iomarket.cpp
    ${CMAKE_CURRENT_LIST_DIR}/item.cpp
    ${CMAKE_CURRENT_LIST_DIR}/items.cpp
    ${CMAKE_CURRENT_LIST_DIR}/luascript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mailbox.cpp
    ${CMAKE_CURRENT_LIST_DIR}/map.cpp
    ${CMAKE_CURRENT_LIST_DIR}/modules.cpp
    ${CMAKE_CURRENT_LIST_DIR}/monster.cpp
    ${CMAKE_CURRENT_LIST_DIR}/monsters.cpp
    ${CMAKE_CURRENT_LIST_DIR}/mounts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/movement.cpp
    ${CMAKE_CURRENT_LIST_DIR}/networkmessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/npc.cpp
    ${CMAKE_CURRENT_LIST_DIR}/outfit.cpp
    ${CMAKE_CURRENT_LIST_DIR}/outputmessage.cpp
    ${CMAKE_CURRENT_LIST_DIR}/party.cpp
    ${CMAKE_CURRENT_LIST_DIR}/player.cpp
    ${CMAKE_CURRENT_LIST_DIR}/position.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocolgame.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocollogin.cpp
    ${CMAKE_CURRENT_LIST_DIR}/protocolstatus.cpp
    ${CMAKE_CURRENT_LIST_DIR}/quests.cpp
    ${CMAKE_CURRENT_LIST_DIR}/raids.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scripts.cpp
    ${CMAKE_CURRENT_LIST_DIR}/server.cpp
    ${CMAKE_CURRENT_LIST_DIR}/signals.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spawn.cpp
    ${CMAKE_CURRENT_LIST_DIR}/spells.cpp
    ${CMAKE_CURRENT_LIST_DIR}/talkaction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tasks.cpp
    ${CMAKE_CURRENT_LIST_DIR}/teleport.cpp
    ${CMAKE_CURRENT_LIST_DIR}/thing.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/tools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/trashholder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/vocation.cpp
    ${CMAKE_CURRENT_LIST_DIR}/waitlist.cpp
    ${CMAKE_CURRENT_LIST_DIR}/weapons.cpp
    ${CMAKE_CURRENT_LIST_DIR}/wildcardtree.cpp
)
target_include_directories(canary_server_lib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)

add_library(canary_lib::server_lib ALIAS canary_server_lib)

target_include_directories(canary_server_lib SYSTEM
                          PUBLIC
                            ${LUA_INCLUDE_DIR}
                            ${CANARY_EXTERNAL_PATH}
                            ${CANARY_LIB_PATH}
)

target_link_libraries(canary_server_lib
                        PUBLIC
                          Boost::boost Threads::Threads mariadbclient
                          canary_lib::common_lib
                        PRIVATE
                          ZLIB::ZLIB pugixml::pugixml Boost::system 
                          Boost::filesystem Boost::iostreams
                          Boost::date_time ${LUA_LIBRARIES}
                          ${PROJECT_WARNINGS_FLAGS}
)

target_compile_options(canary_server_lib PRIVATE ${PROJECT_WARNINGS_FLAGS})


# *****************************************************************************
# Git Version
# *****************************************************************************

# Define the two required variables before including
# the source code for watching a git repository.
set(PRE_CONFIGURE_FILE "../cmake/gitmetadata.h.in")
set(POST_CONFIGURE_FILE "${CMAKE_CURRENT_BINARY_DIR}/gitmetadata.h")
include(GitWatcher)
if (Git_FOUND)
    message("-- Git Found")
    add_dependencies(canary_server_lib check_git)
    target_include_directories(canary_server_lib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()


# *****************************************************************************
# Executable
# *****************************************************************************
add_executable(canary
              ${CMAKE_CURRENT_LIST_DIR}/otserv.cpp
              ${CMAKE_CURRENT_LIST_DIR}/canary-server.rc
              )

target_link_libraries(canary PUBLIC canary_lib::server_lib)

set_target_properties(canary
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
