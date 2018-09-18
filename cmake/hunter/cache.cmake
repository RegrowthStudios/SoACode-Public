set(
    HUNTER_CACHE_SERVERS
    "https://github.com/huntercache/main"
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
message(STATUS "GITHUB_USER_PASSWORD: $ENV{GITHUB_USER_PASSWORD}")
message(STATUS "Password empty: ${password_is_empty}")
message(STATUS "Hunter upload: ${HUNTER_RUN_UPLOAD}")

set(
    HUNTER_PASSWORDS_PATH
    "${CMAKE_CURRENT_LIST_DIR}/cmake/Hunter/passwords.cmake"
    CACHE
    FILEPATH
    "Hunter passwords"
)