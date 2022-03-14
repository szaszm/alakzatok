cmake_minimum_required(VERSION 3.11)
project(alakzatok)
set(CMAKE_CXX_STANDARD 11)
include(FetchContent)

option(SYSTEM_SDL2 "Use system SDL2" OFF)

if(SYSTEM_SDL2)
	find_package(SDL2)
	set(SDL2_libs SDL2::SDL2 SDL2::SDL2main)
else()
	FetchContent_Declare(SDL2
			URL         https://github.com/libsdl-org/SDL/releases/download/release-2.0.20/SDL2-2.0.20.tar.gz
			URL_HASH    SHA256=c56aba1d7b5b0e7e999e4a7698c70b63a3394ff9704b5f6e1c57e0c16f04dd06
	)
	FetchContent_MakeAvailable(SDL2)
	set(SDL2_libs SDL2::SDL2-static SDL2::SDL2main)
endif()

add_executable(alakzatok main.cpp)
target_link_libraries(alakzatok ${SDL2_libs})
