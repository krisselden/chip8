cmake_minimum_required(VERSION 2.8.3)

set(project_name "chip8")
project(${project_name})

# setup version numbers
set(${project_name}_VERSION_MAJOR 0)
set(${project_name}_VERSION_MINOR 0)
set(${project_name}_VERSION_PATCH 0)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules/")

if(WINDOWS OR MSVC)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W3")
else()
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -O -g")
endif()

if(MSVC)
  if(NOT CMAKE_CXX_FLAGS MATCHES "/EHsc")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
  endif()
endif()

#
# MSVC-specific settings
#
# Remove security warnings with Visual C++
#
if(MSVC)
  add_definitions( -D_CRT_SECURE_NO_DEPRECATE )
  add_definitions( -D_CRT_SECURE_NO_WARNINGS )
endif()

#
# Apple-specific stuff
#
if( APPLE )
  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )
  set( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libc++" )
  set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libc++" )
  set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libc++" )
endif( APPLE )

set( CMAKE_INCLUDE_CURRENT_DIR ON )

set( EMULATOR_SOURCE_FILES
  src/chip8/Functions.cpp
  src/chip8/Opcodes.cpp
)

set( HOST_APPLICATION_SOURCE_FILES
  src/host/Main.cpp
  src/host/Application.cpp
  src/host/FileUtilities.cpp
)

set( INCLUDE_DIRS
  ${PROJECT_SOURCE_DIR}/include
)

include_directories( ${INCLUDE_DIRS} )

add_executable( ${project_name} ${HOST_APPLICATION_SOURCE_FILES} ${EMULATOR_SOURCE_FILES} ${INCLUDE_DIRS} )

find_package(SDL2)

if(SDL2_FOUND)
  include_directories( ${SDL2_INCLUDE_DIR} )
  target_link_libraries( ${project_name} ${SDL2_LIBRARY} )
endif(SDL2_FOUND)

#
# Copy content files to output directory
#
add_custom_command(
  TARGET ${project_name}
  POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E echo "Copying content to output directory."
  COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/assets" $<TARGET_FILE_DIR:${project_name}>
)

add_subdirectory(test)