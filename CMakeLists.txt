cmake_minimum_required(VERSION 3.16)
project(sonify VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)

# -----------------------------
# Library target
# -----------------------------

set(LIB_SOURCES
  src/utils.cpp
)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

# -----------------------------
# Executable target
# -----------------------------

# Raylib build options
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_WAYLAND ON CACHE BOOL "" FORCE)


add_subdirectory(external/raylib)

set(SOURCES
  src/main.cpp
  src/Sonify.cpp
  src/DTexture.cpp
  src/utils.cpp
  src/LineItem.cpp
  src/CircleItem.cpp
  src/PathItem.cpp
  src/PixelMapManager.cpp

  include/sonify/DefaultPixelMappings/IntensityMap.hpp
  include/sonify/DefaultPixelMappings/HSVMap.hpp
)

set(HEADERS
    src/Config.hpp

)

add_executable(${PROJECT_NAME}_app ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}_app raylib)

# Include directories
target_include_directories(${PROJECT_NAME}_app PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

# -----------------------------
# Installation
# -----------------------------
# Install library
install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Install headers
install(DIRECTORY include/${PROJECT_NAME}
  DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}
)

# Install app binary
install(TARGETS ${PROJECT_NAME}_app
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  RENAME ${PROJECT_NAME}
)

# Export CMake package
install(EXPORT ${PROJECT_NAME}-targets
  FILE ${PROJECT_NAME}-config.cmake
  NAMESPACE ${PROJECT_NAME}::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME})
