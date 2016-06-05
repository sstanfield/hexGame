cmake_minimum_required(VERSION 3.0.2)
project(hexGame)


#include(CheckCCompilerFlag)
#CHECK_C_COMPILER_FLAG("-std=c11" COMPILER_SUPPORTS_C11)
#if(COMPILER_SUPPORTS_C11)
#    set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c11")
#else()
#    message(STATUS "The compiler ${CMAKE_C_COMPILER} has no C11 support. Please use a different C compiler.")
#endif()
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
#set(CMAKE_C_FLAGS "${CMAKE_CXX_FLAGS} -std=c99")

find_package(OpenGL REQUIRED)

#set(SOURCE_FILES main.cpp)
file(GLOB SOURCE_FILES
	src/*.c src/*.h
	src/state/*.c src/state/*.h 
	src/util/*.c src/util/*.h
	src/render/*.c src/render/*.h
	src/render/pc/glew.c
)


#list(APPEND EXTRA_LIBS pthread)
list(APPEND EXTRA_LIBS X11)
list(APPEND EXTRA_LIBS m)
list(APPEND EXTRA_LIBS ${OPENGL_LIBRARY})
list(APPEND EXTRA_LIBS png)
list(APPEND EXTRA_LIBS freetype)
#list(APPEND EXTRA_LIBS GLEW)

include_directories(/usr/include/freetype2)

add_executable(hexGame ${SOURCE_FILES})

target_link_libraries(hexGame ${EXTRA_LIBS})

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
