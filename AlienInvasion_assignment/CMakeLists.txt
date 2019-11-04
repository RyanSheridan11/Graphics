cmake_minimum_required(VERSION 2.8)

project(assignment1)

add_executable(Castle.out Castle.cpp)



find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )

target_link_libraries( Castle.out ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

