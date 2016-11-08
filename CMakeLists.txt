cmake_minimum_required(VERSION 3.4.3)
#cmake_minimum_required(VERSION 3.0.2)
project(hexGame)

find_package(OpenGL REQUIRED)
list(APPEND EXTRA_LIBS ${OPENGL_LIBRARY})

if(${CMAKE_CXX_COMPILER_ID} MATCHES "GNU" OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -Wall -Wextra -Wno-unused-parameter")
	SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -std=c++14 -Wall -Wextra -Wno-unused-parameter")
	SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS_DEBUG} -O3 -Wall -Wextra -Wno-unused-parameter")
	SET(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -O0 -Wall -Wextra -Wno-unused-parameter")
endif()

file(GLOB SOURCE_FILES
	src/*.cpp src/*.c src/*.h
	src/state/*.cpp src/state/*.c src/state/*.h 
	src/util/*.cpp src/util/*.c src/util/*.h
	src/render/*.cpp src/render/*.c src/render/*.h
	src/gui/*.cpp
	src/gui/tb/*.cpp
	src/gui/tb/image/*.cpp
	src/gui/tb/animation/*.cpp
	src/gui/tb/parser/*.cpp
	src/gui/tb/renderers/*.cpp
	src/gui/tb/utf8/*.cpp
	src/lua/*.c
)

IF(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_GLFW=3 -s DISABLE_EXCEPTION_CATCHING=0 -s TOTAL_MEMORY=512000000 -s ASSERTIONS=1")
	SET(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "${CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO} --preload-file ../assets/@/assets")
	SET(CMAKE_EXECUTABLE_SUFFIX ".html")
	# Make TurboBadger happy.
	add_definitions(-DTB_CLIPBOARD_GLFW)
	add_definitions(-DTB_FILE_POSIX)
	add_definitions(-DTB_SYSTEM_LINUX)
ELSE(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")
	add_subdirectory(glfw)
	list(APPEND SOURCE_FILES src/render/pc/glew.c)
	include_directories(glfw/include)
	include_directories(src/render/pc)
	list(APPEND EXTRA_LIBS glfw)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Emscripten")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	list(APPEND EXTRA_LIBS pthread)
	list(APPEND EXTRA_LIBS X11)
	list(APPEND EXTRA_LIBS m)
	list(APPEND EXTRA_LIBS png)
	list(APPEND EXTRA_LIBS jpeg)
	list(APPEND EXTRA_LIBS freetype)
	set (CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")
	include_directories(/usr/include/freetype2)

	add_definitions(-D_P_LINUX)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	# Remove exception handling
	#STRING(REPLACE "/EHsc" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	# Remove rtti
	#STRING(REPLACE "/GR" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 /D_UNICODE /DUNICODE /D_WINDOWS /c /GR-")
	add_definitions(-D_P_WINDOWS)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Windows")

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	add_definitions(-D_P_OSX)
	add_definitions(-DMACOSX)
ENDIF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

list(APPEND SOURCE_FILES ${PLATFORM_SOURCE_FILES})

include_directories(src)
include_directories(src/include)
include_directories(src/gui/tb)

add_executable(hexGame ${SOURCE_FILES})

target_link_libraries(hexGame ${EXTRA_LIBS})

set( CMAKE_EXPORT_COMPILE_COMMANDS 1 )
