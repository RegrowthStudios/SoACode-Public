cmake_minimum_required (VERSION 3.0)

include(${CMAKE_CURRENT_LIST_DIR}/cmake/hunter/HunterGate.cmake)

HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.208.tar.gz"
    SHA1 "4128ac8c79c21b250bf825e8119ce0bff05e5132"
)

project(SoA)

add_subdirectory(Vorb)
add_subdirectory(SoA)



