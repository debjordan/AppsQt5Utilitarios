cmake_minimum_required(VERSION 3.16)
project(HardwareMonitor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(HardwareMonitor
    main.cpp
    mainwindow.cpp
    systeminfo.cpp
)

target_link_libraries(HardwareMonitor Qt5::Core Qt5::Widgets)
