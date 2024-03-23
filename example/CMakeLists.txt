cmake_minimum_required(VERSION 3.8)
project(tinydi-examples)

if(NOT TARGET tinydi)
  find_package(tinydi CONFIG REQUIRED)
endif()

add_executable(example-default-arg default-arg.cpp)
target_link_libraries(example-default-arg tinydi::tinydi)
target_compile_options(example-default-arg PRIVATE -Wall -Wextra -Werror -Weffc++ -Wold-style-cast)
