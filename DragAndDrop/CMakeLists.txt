cmake_minimum_required(VERSION 3.25)

project(moving)

find_package(SDL3 REQUIRED)
find_package(SDL3_image REQUIRED)

add_executable(${PROJECT_NAME} src/main.c)

target_link_libraries(${PROJECT_NAME} PRIVATE SDL3::SDL3 SDL3_image::SDL3_image)