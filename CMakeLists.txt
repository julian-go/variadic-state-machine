set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
cmake_minimum_required(VERSION 3.22)

cmake_policy(VERSION 3.22)

project(
  vsm
  VERSION 1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)

set(EXAMPLE_SRC 
example/main.cpp
example/states.cpp
)

add_executable(example ${EXAMPLE_SRC})

target_include_directories(example PRIVATE include/)