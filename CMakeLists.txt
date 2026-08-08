cmake_minimum_required(VERSION 4.3.1)

project(Example)
add_executable(Example MECA/Example1.cpp)

target_include_directories(Example PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/MECA
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger
)