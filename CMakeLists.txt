cmake_minimum_required(VERSION 3.0.2)
project(hexGame)

add_subdirectory(glfw)

find_package(OpenGL REQUIRED)
list(APPEND EXTRA_LIBS ${OPENGL_LIBRARY})

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11 -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -std=c++11 -fno-exceptions -fno-rtti -Wall -Wextra -Wno-unused-parameter")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -O3 -Wall -Wextra -Wno-unused-parameter")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -Wextra -Wno-unused-parameter")
endif()

file(GLOB SOURCE_FILES
	src/*.cpp src/*.c src/*.h
	src/state/*.cpp src/state/*.c src/state/*.h 
	src/util/*.cpp src/util/*.c src/util/*.h
	src/render/*.cpp src/render/*.c src/render/*.h
	src/render/pc/glew.c
	src/gui/*.cpp
	src/gui/tb/*.cpp
	src/gui/tb/image/*.cpp
	src/gui/tb/animation/*.cpp
	src/gui/tb/parser/*.cpp
	src/gui/tb/renderers/*.cpp
	src/gui/tb/utf8/*.cpp
	src/lua/*.c
)


IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND EXTRA_LIBS pthread)
	list(APPEND EXTRA_LIBS X11)
	#list(APPEND EXTRA_LIBS X11-xcb)
	#list(APPEND EXTRA_LIBS xcb)
	#list(APPEND EXTRA_LIBS xcb-randr)
	list(APPEND EXTRA_LIBS m)
	list(APPEND EXTRA_LIBS png)

	#list(APPEND EXTRA_LIBS Xxf86vm)
	#list(APPEND EXTRA_LIBS "${X11_Xinerama_LIB}")
	#list(APPEND EXTRA_LIBS "${X11_Xcursor_LIB}")
	#list(APPEND EXTRA_LIBS "${X11_Xrandr_LIB}")
	list(APPEND EXTRA_LIBS jpeg)
	list(APPEND EXTRA_LIBS freetype)
	set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
	include_directories(/usr/include/freetype2)

	add_definitions(-D_P_LINUX)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Remove exception handling
	STRING(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# Remove rtti
	STRING(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 /D_UNICODE /DUNICODE /D_WINDOWS /c /GR-")
	add_definitions(-D_P_WINDOWS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions(-D_P_OSX)
	add_definitions(-DMACOSX)
	add_definitions(-Dnullptr=0)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

list(APPEND SOURCE_FILES ${PLATFORM_SOURCE_FILES})

include_directories(src)
include_directories(glfw/include)
include_directories(src/render/pc)
include_directories(src/gui/tb)

add_executable(hexGame ${SOURCE_FILES})

target_link_libraries(hexGame ${EXTRA_LIBS} glfw)

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
