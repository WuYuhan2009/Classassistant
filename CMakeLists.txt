cmake_minimum_required(VERSION 3.10)
project(Classassistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.12 REQUIRED COMPONENTS Core Gui Widgets Svg Network)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/floatingball.cpp
    src/settingsdialog.cpp
    src/attendancewidget.cpp
    src/randomnamedialog.cpp
    src/welcomedialog.cpp
    src/configmanager.cpp
    src/utils.cpp
)

set(HEADERS
    src/mainwindow.h
    src/floatingball.h
    src/settingsdialog.h
    src/attendancewidget.h
    src/randomnamedialog.h
    src/welcomedialog.h
    src/buttonconfig.h
    src/configmanager.h
    src/utils.h
)

set(UIS
    ui/mainwindow.ui
    ui/settingsdialog.ui
    ui/welcomedialog.ui
    ui/randomnamedialog.ui
)

set(RESOURCES
    resources/resources.qrc
)

add_executable(Classassistant ${SOURCES} ${HEADERS} ${UIS} ${RESOURCES})

target_link_libraries(Classassistant PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::Svg Qt5::Network)

target_include_directories(Classassistant PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
