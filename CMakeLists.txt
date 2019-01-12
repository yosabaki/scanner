cmake_minimum_required(VERSION 2.8)

project(scanner)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(Qt5Widgets REQUIRED)

add_executable(scanner main.cpp mainwindow.h mainwindow.cpp scanner.h scanner.cpp custommodel.h custommodel.cpp)
qt5_use_modules(scanner Widgets)
