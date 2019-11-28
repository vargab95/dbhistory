cmake_minimum_required (VERSION 3.5)

find_package (SQLite3 REQUIRED)
include_directories(${SQLITE3_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_executable (dbhistory
                ${CMAKE_CURRENT_SOURCE_DIR}/main.c
                ${CMAKE_CURRENT_SOURCE_DIR}/daemon.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/db_handler.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/db_common.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/db_read.c
                ${CMAKE_CURRENT_SOURCE_DIR}/db/db_read_re.c
                ${CMAKE_CURRENT_SOURCE_DIR}/sqlite_wrapper.c
                ${CMAKE_CURRENT_SOURCE_DIR}/utils.c
                ${CMAKE_CURRENT_SOURCE_DIR}/client.c)
target_link_libraries (dbhistory ${SQLITE3_LIBRARIES})
