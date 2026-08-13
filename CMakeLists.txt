cmake_minimum_required(VERSION 4.3.1)

add_library(meca_logger INTERFACE)
target_include_directories(meca_logger INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/MECA
    ${CMAKE_CURRENT_SOURCE_DIR}/Logger
)

project(Example)
add_executable(Example1 MECA/Example1.cpp)
target_link_libraries(Example1 PRIVATE meca_logger)

add_executable(Example2 MECA/Example2.cpp)
target_link_libraries(Example2 PRIVATE meca_logger)