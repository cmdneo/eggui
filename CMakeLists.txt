cmake_minimum_required(VERSION 3.15)
project(EGGUI DESCRIPTION "A simple UI library")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall -Wextra -Wnull-dereference -Wshadow -Wformat=2 -pedantic)
# Debug mode sanitizers
add_compile_options("$<$<CONFIG:Debug>:-fsanitize=address,undefined>")
add_link_options("$<$<CONFIG:Debug>:-fsanitize=address,undefined>")

include_directories("${CMAKE_SOURCE_DIR}/include/eggui")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_library(eggui
	src/window.cxx
	src/managers.cxx
	src/canvas.cxx
	src/graphics.cxx
	src/widget.cxx
	src/container.cxx
	src/button.cxx
	src/scrollable.cxx
)

# Add examples
add_executable(test_main examples/some_test.cxx)
target_link_libraries(test_main eggui raylib m)
