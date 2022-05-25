cmake_minimum_required(VERSION 3.13)
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread -lsfml-system -lsfml-network")

project(AlphaTracker)
find_package(OpenCV REQUIRED)

include_directories(
        ${CMAKE_CURRENT_LIST_DIR}
        ${OpenCV_INCLUDE_DIRS}
        include
)

#add_library(
#        gui
#        src/gui.cc
#        include/gui.h
#)
#add_library(
#        stack
#        src/stack.cc
#        include/stack.h
#)
#add_library(
#        functions
#        src/functions.cc
#        include/functions.h
#)
#add_library(
#        console
#        src/console.cc
#        include/console.h
#)
#add_library(
#        settings
#        src/settings.cc
#        include/settings.h
#)

add_executable(
        main
        src/main.cc
        src/gui.cc include/gui.h
        src/stack.cc include/stack.h
        src/functions.cc include/functions.h
        src/console.cc include/console.h
        src/settings.cc include/settings.h
        include/common.h src/common.cc)
target_link_libraries(main ${OpenCV_LIBS})