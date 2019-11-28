cmake_minimum_required (VERSION 3.5)

set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR}/install)
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
# set(CMAKE_C_CLANG_TIDY "clang-tidy;bugprone-*,cert-*,cppcoreguidelines-*,clang-analyzer-*,misc-*,modernize-*,mpi-*,performace-*,portability-*,readability-*")
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

option(DBHISTORY_BUILD_TESTS "Switch to build test or not")

option(DBHISTORY_USE_DAEMON "")
IF(DBHISTORY_USE_DAEMON)
    ADD_DEFINITIONS(-DDBHISTORY_USE_DAEMON)
ENDIF(DBHISTORY_USE_DAEMON)

option(DBHISTORY_USE_REGEX "")
IF(DBHISTORY_USE_REGEX)
    ADD_DEFINITIONS(-DDBHISTORY_USE_REGEX)
ENDIF(DBHISTORY_USE_REGEX)

add_subdirectory (src)
if (DBHISTORY_BUILD_TESTS)
  add_subdirectory (test)
endif ()
enable_testing ()

