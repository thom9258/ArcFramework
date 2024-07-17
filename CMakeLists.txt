cmake_minimum_required(VERSION 3.1)

if(PROJECT_IS_TOP_LEVEL)
    message("====================================")
    message("Arc Game Framework")
    message("---")
    message("====================================")
endif()

if(MSVC)
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

project(ArcFramework VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/glm)
find_package(SDL2 REQUIRED)

# https://vulkan.lunarg.com/doc/view/latest/linux/getting_started_ubuntu.html
find_package(Vulkan REQUIRED)

set(ARC_SOURCES 
  ${CMAKE_CURRENT_SOURCE_DIR}/src/Context.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/BasicBuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/VertexBuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/IndexBuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/UniformBuffer.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/src/FlightFrame.cpp
)
    
set(ARC_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/Context.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/BasicBuffer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/VertexBuffer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/IndexBuffer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/UniformBuffer.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/FlightFrame.hpp
)

add_library(${PROJECT_NAME} STATIC)

target_sources(${PROJECT_NAME} PRIVATE ${ARC_SOURCES})

include_directories(${PROJECT_NAME} ${SDL2_INCLUDE_DIRS} ${Vulkan_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC glm ${SDL2_LIBRARIES} Vulkan::Vulkan)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>"
    "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)
