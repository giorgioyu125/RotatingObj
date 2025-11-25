cmake_minimum_required(VERSION 3.16)
project(mygame C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

find_package(OpenGL REQUIRED)

add_subdirectory(include/external-libs/SDL EXCLUDE_FROM_ALL)
add_subdirectory(include/external-libs/cglm EXCLUDE_FROM_ALL)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_library(glad STATIC src/libs/glad.c)
target_include_directories(glad PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

add_executable(mygame_app
    src/main.c
    src/libs/mesh.c
    src/libs/texture.c
    src/libs/transform.c
    src/libs/render.c
    src/libs/camera.c
)

target_include_directories(mygame_app PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
)

target_compile_options(mygame_app PRIVATE -Wall -Wextra -Wpedantic)

if(EXISTS "${PROJECT_SOURCE_DIR}/src/shaders")
  add_custom_command(
    TARGET mygame_app
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:mygame_app>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            "${PROJECT_SOURCE_DIR}/src/shaders"
            "$<TARGET_FILE_DIR:mygame_app>/shaders"
    COMMENT "Copying shaders to build directory"
  )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/assets")
  add_custom_command(
    TARGET mygame_app
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
            "$<TARGET_FILE_DIR:mygame_app>/assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
            "${PROJECT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:mygame_app>/assets"
    COMMENT "Copying assets to build directory"
  )
endif()

target_link_libraries(mygame_app PRIVATE
    SDL3::SDL3
    glad
    OpenGL::GL
    cglm
    m
)
