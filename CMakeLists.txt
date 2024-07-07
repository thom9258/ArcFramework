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
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/error.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/window.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/math.cpp
#   ${CMAKE_CURRENT_SOURCE_DIR}/src/shader.cpp
)
    
set(ARC_INCLUDES
  ${CMAKE_CURRENT_SOURCE_DIR}/arc/Context.hpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/arc/sdlgl.hpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/arc/error.hpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/arc/shader.hpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/arc/window.hpp
#  ${CMAKE_CURRENT_SOURCE_DIR}/arc/math.hpp
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
