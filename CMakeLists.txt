cmake_minimum_required(VERSION 3.10)
project(Classassistant LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 5.12 REQUIRED COMPONENTS Widgets Gui Core)

set(SOURCES
    src/main.cpp
    src/appcontroller.cpp
    src/configmanager.cpp
    src/resourcedownloader.cpp
    src/sidebarwindow.cpp
    src/floatingballwindow.cpp
    src/attendanceoverlay.cpp
    src/settingsdialog.cpp
    src/welcomedialog.cpp
    src/custombuttondialog.cpp
    src/randomnamedialog.cpp
)

set(HEADERS
    src/include/appcontroller.h
    src/include/configmanager.h
    src/include/models.h
    src/include/resourcedownloader.h
    src/include/sidebarwindow.h
    src/include/floatingballwindow.h
    src/include/attendanceoverlay.h
    src/include/settingsdialog.h
    src/include/welcomedialog.h
    src/include/custombuttondialog.h
    src/include/randomnamedialog.h
)

add_executable(Classassistant ${SOURCES} ${HEADERS})
target_include_directories(Classassistant PRIVATE src/include)
target_link_libraries(Classassistant PRIVATE Qt5::Widgets Qt5::Gui Qt5::Core)

if(WIN32)
  target_link_libraries(Classassistant PRIVATE shell32)
endif()
