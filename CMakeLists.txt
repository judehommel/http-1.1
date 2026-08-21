cmake_minimum_required(VERSION 3.31.0)
project(http/1.1 VERSION 0.1.0)
cmake_policy(SET CMP0072 NEW)

add_compile_options(-Wall -Werror -Wpedantic)

set(SOURCE_FILES ./src/main.c ./src/dataFlowHandler.c ./src/utils.c)

add_executable(httpserver ${SOURCE_FILES})
target_include_directories(httpserver PRIVATE include)
target_link_libraries(httpserver m)

