cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  if (APPLE)
    message("\nRun the build script:  build.sh\n")
  endif (APPLE)
  message(FATAL_ERROR "You don't want to configure in the source folder!")
endif()


project("SoA")
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

enable_testing()

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU" OR
    "${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
    set(warnings "-Wall -Wextra -Werror")
    ADD_DEFINITIONS(
        -std=c++11
        -std=c++0x
        # Other flags
        ${warnings}
    )
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
    set(warnings "/W4 /WX /EHsc")
endif()

IF(APPLE)
   INCLUDE_DIRECTORIES ( /System/Library/Frameworks )
   FIND_LIBRARY(COCOA_LIBRARY Cocoa)
   FIND_LIBRARY(GLUT_LIBRARY GLUT )
   FIND_LIBRARY(OpenGL_LIBRARY OpenGL )
   MARK_AS_ADVANCED (COCOA_LIBRARY
                     GLUT_LIBRARY
                     OpenGL_LIBRARY)
   SET(EXTRA_LIBS ${COCOA_LIBRARY} ${GLUT_LIBRARY} ${OpenGL_LIBRARY})
ENDIF (APPLE)

# Open GL
find_package(OpenGL)
if (OPENGL_FOUND)
   include_directories(SYSTEM ${OPENGL_INCLUDE_DIRS})
endif (OPENGL_FOUND)

# SDL
find_package(SDL)
if (SDL_FOUND)
   include_directories(SYSTEM ${SDL_INCLUDE_DIRS})
endif (SDL_FOUND)

# Glew
find_package(GLEW)
if (GLEW_FOUND)
   include_directories(SYSTEM ${GLEW_INCLUDE_DIRS})
endif (GLEW_FOUND)

# BOOST filesystem
set(Boost_USE_STATIC_LIBS OFF)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
set(BOOST_VERSION 1.55.0)
set(BOOST_COMPONENTS filesystem)
find_package(Boost ${BOOST_VERSION} COMPONENTS ${BOOST_COMPONENTS} REQUIRED)
if (Boost_FOUND)
   include_directories(SYSTEM ${BOOST_INCLUDE_DIRS})
endif (Boost_FOUND)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/include)

#lodepng
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/lodepng)

# readerwriterqueue
include_directories(SYSTEM ${CMAKE_CURRENT_SOURCE_DIR}/../deps/readerwriterqueue)

set(extra_sources
    ${CMAKE_CURRENT_SOURCE_DIR}/../deps/lodepng/lodepng.cpp
    )

file(GLOB engine_files
    "*.h"
    "*.cpp"
)

set(skip_these_for_now
)

foreach(eng_file in ${skip_these_for_now})
  list(REMOVE_ITEM engine_files ${CMAKE_CURRENT_SOURCE_DIR}/${eng_file})
endforeach()

# doxygen
find_package(Doxygen)
option(BUILD_DOCUMENTATION "Create and install the HTML based API documentation (requires Doxygen)" ${DOXYGEN_FOUND})

if(BUILD_DOCUMENTATION)
    if(NOT DOXYGEN_FOUND)
        message(FATAL_ERROR "Doxygen is needed to build the documentation.")
    endif()

    set(doxyfile_in ${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in)
    set(doxyfile ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)

    configure_file(${doxyfile_in} ${doxyfile} @ONLY)

    add_custom_target(doc
        COMMAND ${DOXYGEN_EXECUTABLE} ${doxyfile}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM)

    install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/html DESTINATION share/doc)
endif()

add_executable(soa ${engine_files} ${extra_sources})
target_link_libraries(soa ${EXTRA_LIBS}
    ${OPENGL_INCLUDE_DIRS}
    ${SDL_INCLUDE_DIRS}
    ${GLEW_INCLUDE_DIRS}
    ${BOOST_INCLUDE_DIRS})
