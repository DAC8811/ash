cmake_minimum_required(VERSION 3.0.0)
project(ash VERSION 0.1.0)

add_executable(ash src/main.cpp)
target_include_directories(ash PRIVATE include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

