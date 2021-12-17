cmake_minimum_required (VERSION 3.5)

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
                ${CMAKE_CURRENT_SOURCE_DIR}/db/sqlite_wrapper.c
                ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
                ${CMAKE_CURRENT_SOURCE_DIR}/client.c
                ${CMAKE_CURRENT_SOURCE_DIR}/config.c)

target_link_libraries (dbhistory ${SQLITE3_LIBRARIES})

install(TARGETS dbhistory DESTINATION /usr/bin)
