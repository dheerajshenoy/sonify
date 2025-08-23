cmake_minimum_required(VERSION 3.16)
project(sonify VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

set(SOURCES
  src/main.cpp
  src/Sonify.cpp
  src/DTexture.cpp
  src/utils.cpp
  src/LineItem.cpp
)

set(HEADERS
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME} raylib sonifycpp)
