cmake_minimum_required(VERSION 3.1)
project(window)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
set(CMAKE_CXX_STANDARD 20)


set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
file(COPY 
  ${CMAKE_CURRENT_BINARY_DIR}/compile_commands.json
  ${CMAKE_CURRENT_SOURCE_DIR}/compile_commands.json
)

add_executable(${PROJECT_NAME} main.cpp)

add_subdirectory(
  ${CMAKE_CURRENT_SOURCE_DIR}/../../ 
  ${CMAKE_CURRENT_SOURCE_DIR}/ArcFramework
)
target_link_libraries(${PROJECT_NAME} PRIVATE ArcFramework)
