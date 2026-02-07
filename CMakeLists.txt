cmake_minimum_required(VERSION 3.10)

project(ClassAssistant VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)
if(WIN32)
    find_package(Qt5 COMPONENTS WinExtras REQUIRED)
endif()

set(PROJECT_SOURCES
    src/main.cpp
    src/Utils.h
    src/Utils.cpp
    src/ui/Sidebar.h
    src/ui/Sidebar.cpp
    src/ui/FloatingBall.h
    src/ui/FloatingBall.cpp
    src/ui/Tools.h
    src/ui/Tools.cpp
    resources.qrc
)

add_executable(ClassAssistant ${PROJECT_SOURCES})

target_link_libraries(ClassAssistant PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Network)
if(WIN32)
    target_link_libraries(ClassAssistant PRIVATE Qt5::WinExtras)
endif()

file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
