cmake_minimum_required(VERSION 3.0.0)
project(ash VERSION 0.1.0)

set(CXX_FLAGS 
    -pthread)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_FLAGS}")

aux_source_directory(${PROJECT_SOURCE_DIR}/src/muduo DIR_MUDUO_SRCS)
aux_source_directory(${PROJECT_SOURCE_DIR}/src/network DIR_NETWORK_SRCS)
add_executable(ash src/main.cpp ${DIR_MUDUO_SRCS} ${DIR_NETWORK_SRCS})
target_include_directories(ash PRIVATE include)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})

