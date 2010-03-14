cmake_minimum_required(VERSION 2.6.4)

execute_process( COMMAND flusspferd --cmake OUTPUT_VARIABLE hippo_cmake) 
include("${hippo_cmake}")


if(NOT CMAKE_BUILD_TYPE)
    set( CMAKE_BUILD_TYPE Release CACHE STRING
        "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel."
        FORCE)
endif()

FILE(GLOB_RECURSE sources 
     "${CMAKE_SOURCE_DIR}/src/*.cpp" 
     "${CMAKE_SOURCE_DIR}/src/*.hpp")

add_definitions("-Wall")
flusspferd_plugin(
  "zest"
  SOURCES ${sources})
