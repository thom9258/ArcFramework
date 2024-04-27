cmake_minimum_required(VERSION 3.1)
project(ArcFramework VERSION 0.1.0)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -ggdb")
set(CMAKE_CXX_STANDARD 20)

# Generate compile_commands.json
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(THIRDPARTY_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/3rdparty)
if(EXISTS ${THIRDPARTY_ROOT})
message("3rdparty root: " ${THIRDPARTY_ROOT})
else()
message("3rdparty root: <not found>")
endif()

set(ARC_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/arc/)

set(ARC_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/error.cpp
)

#====================================
# SDL2

find_package(SDL2 REQUIRED)

message("SDL2 include:  " ${SDL2_INCLUDE_DIRS})
message("SDL2 library:  " ${SDL2_LIBRARIES})

#====================================
# OpenGL & GLM


set(GLAD_INCLUDE_DIRS ${THIRDPARTY_ROOT}/glad/include)
set(GLAD_SOURCES    ${THIRDPARTY_ROOT}/glad/src/glad.c)

message("GLAD include dirs: " ${GLAD_INCLUDE_DIRS})
message("GLAD sources:      " ${GLAD_SOURCES})

set(GLM_INCLUDE_DIRS ${THIRDPARTY_ROOT}/glm/)
message("GLM include dirs:  " ${GLM_INCLUDE_DIRS})


#====================================
# Make Library


# Create Library from sources
add_library(${PROJECT_NAME} STATIC
                            ${ARC_SOURCES}
                            ${GLAD_SOURCES}
)

target_link_libraries(${PROJECT_NAME}
    -lGL -lX11 -ldl -lpthread -lrt
)

include_directories  (${PROJECT_NAME} ${GLM_INCLUDE_DIRS})
include_directories  (${PROJECT_NAME} ${GLAD_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${GLAD_LIBRARIES})

include_directories(${PROJECT_NAME} ${THIRDPARTY_ROOT})
