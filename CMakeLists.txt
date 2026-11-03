cmake_minimum_required(VERSION 3.15)
project(main C)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

find_package(SDL2 REQUIRED)
add_executable(chip-8 main.c window.c cpu.c input.c)

if(TARGET SDL2::SDL2)
	target_link_libraries(chip-8 PRIVATE SDL2::SDL2)
else()
	target_include_directories(chip-8 PRIVATE ${SDL2_INCLUDE_DIRS})
	target_link_libraries(chip-8 PRIVATE ${SDL2_LIBRARIES})
endif()

if(MSVC)
	target_compile_options(chip-8 PRIVATE /W4)
else()
	target_compile_options(chip-8 PRIVATE -Wall -Wextra -pedantic)
endif()