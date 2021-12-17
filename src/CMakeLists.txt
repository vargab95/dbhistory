cmake_minimum_required (VERSION 3.5)

project(dbhistory)

include(ExternalProject)

ExternalProject_Add(inih
                    GIT_REPOSITORY https://github.com/benhoyt/inih.git
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/inih
                    SOURCE_DIR        "${CMAKE_BINARY_DIR}/inih-src"
                    BINARY_DIR        "${CMAKE_BINARY_DIR}/inih-build"
                    CONFIGURE_COMMAND ""
                    BUILD_COMMAND     ""
                    INSTALL_COMMAND   ""
                    TEST_COMMAND      "")
include_directories(${CMAKE_BINARY_DIR}/inih-src/)

find_package (SQLite3 REQUIRED)

include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable (dbhistory
                ${CMAKE_CURRENT_SOURCE_DIR}/main.c
                ${CMAKE_CURRENT_SOURCE_DIR}/cleaner.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/handler.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/common.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/read.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/path.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/read_re.c
                ${CMAKE_CURRENT_SOURCE_DIR}/sqlite_wrapper.c
                ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
                ${CMAKE_CURRENT_SOURCE_DIR}/client.c
                ${CMAKE_CURRENT_SOURCE_DIR}/config.c
                ${CMAKE_BINARY_DIR}/inih-src/ini.c)

add_dependencies(dbhistory inih)

target_link_libraries (dbhistory ${SQLITE3_LIBRARIES})

install(TARGETS dbhistory DESTINATION /usr/bin)
