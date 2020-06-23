# *****************************************************************************
# Cmake Features
# *****************************************************************************

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Global dá erro no spdlog
# Avoid changes by cmake/make on the source tree
# https://gitlab.kitware.com/cmake/cmake/issues/18403
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# Make will print more details
set(CMAKE_VERBOSE_MAKEFILE OFF)

# *****************************************************************************
# Includes
# *****************************************************************************
include(OptionsCanary)

# *****************************************************************************
# Find Packages
# *****************************************************************************
find_package(Threads REQUIRED)
find_package(Boost 1.53.0 COMPONENTS system filesystem iostreams date_time REQUIRED)
find_package(Lua REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GMP REQUIRED)


# *****************************************************************************
# Library
# *****************************************************************************
add_library(canary_lib "")
target_sources(canary_lib
  PUBLIC
    ${CMAKE_CURRENT_LIST_DIR}/otpch.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/otserv.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/rsa.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scheduler.cpp
    ${CMAKE_CURRENT_LIST_DIR}/scriptmanager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/script.cpp
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
target_include_directories(canary_lib
  PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>)
add_library(canary_lib::canary_lib ALIAS canary_lib)

target_include_directories(canary_lib SYSTEM
                          PUBLIC
                            ${LUA_INCLUDE_DIR}
                            ${CANARY_EXTERNAL_PATH}
                            ${GMP_INCLUDE_DIR}
)

target_link_libraries(canary_lib
                        PUBLIC
                          ZLIB::ZLIB Boost::boost ${GMP_LIBRARIES}
                          Threads::Threads mariadbclient pugixml::pugixml
                          spdlog::spdlog ${LUA_LIBRARIES}
                        PRIVATE
                          Boost::system Boost::filesystem Boost::iostreams
                          Boost::date_time doctest::doctest
)


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
    add_dependencies(canary_lib check_git)
    target_include_directories(canary_lib PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif()


# *****************************************************************************
# Executable
# *****************************************************************************
add_executable(canary otserv.cpp)


# *****************************************************************************
# canary build options
# *****************************************************************************
target_link_libraries(canary PRIVATE canary_lib::canary_lib)
set_target_properties(canary
    PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
)
# target_compile_definitions(canary PRIVATE UNIT_TESTING=1 -fvar-tracking-assignments)
