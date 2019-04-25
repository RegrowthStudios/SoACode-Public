cmake_minimum_required (VERSION 3.0)

set(HUNTER_STATUS_DEBUG ON)
#setup cache server and suppot upload
set(
    HUNTER_CACHE_SERVERS
    "https://github.com/huntercache/SoA"
    CACHE
    STRING
    "Default cache server"
)

string(COMPARE EQUAL "$ENV{TRAVIS}" "true" is_travis)
string(COMPARE EQUAL "$ENV{APPVEYOR}" "True" is_appveyor)
string(COMPARE EQUAL "$ENV{GITHUB_USER_PASSWORD}" "" password_is_empty)

if((is_travis OR is_appveyor) AND NOT password_is_empty)
  option(HUNTER_RUN_UPLOAD "Upload cache binaries" ON)
endif()

message(STATUS "Travis: ${is_travis}")
message(STATUS "Appveyor: ${is_appveyor}")
message(STATUS "Password empty: ${password_is_empty}")
message(STATUS "Hunter upload: ${HUNTER_RUN_UPLOAD}")


set(
    HUNTER_PASSWORDS_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake/hunter/passwords.cmake"
    CACHE
    FILEPATH
    "Hunter passwords"
)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.23.164.tar.gz"
    SHA1 "7bcf092561389dbd70edb3fc228126ae4605e10c"
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

project(SoA)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
  if(MSVC_VERSION GREATER 1900)
    message(FATAL_ERROR "Only Visual Studio 2015 and below are currently supported.")
  endif()
endif()

add_subdirectory(Vorb)
add_subdirectory(SoA)



