cmake_minimum_required(VERSION 3.18)
project(Lab VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
find_package(GLUT REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

add_executable(surface water.cpp)
target_link_libraries(surface PRIVATE ${GLUT_LIBRARIES} /usr/lib/libSOIL.so ${OPENGL_LIBRARIES} glm::glm)

add_executable(naive naive.cpp)
target_link_libraries(naive PRIVATE ${GLUT_LIBRARIES} /usr/lib/libSOIL.so ${OPENGL_LIBRARIES} glm::glm)