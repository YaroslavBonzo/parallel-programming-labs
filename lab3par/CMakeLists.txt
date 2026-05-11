cmake_minimum_required(VERSION 3.12)

project("lab1" CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(${PROJECT_NAME}  "generate.cpp" "main.cpp")

