cmake_minimum_required(VERSION 3.10)

project(ClassAssistant VERSION 1.0.0 LANGUAGES CXX)

# ==============================
# 基础设置
# ==============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ==============================
# ⭐ Win7 兼容关键（必须）
# ==============================
if(MSVC)
    add_definitions(-D_WIN32_WINNT=0x0601)
    add_definitions(-DWINVER=0x0601)
endif()

# ==============================
# 查找 Qt
# ==============================
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

if(WIN32)
    find_package(Qt5 COMPONENTS WinExtras REQUIRED)
endif()

# ==============================
# 源文件
# ==============================
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

# ==============================
# 创建可执行程序
# WIN32 = GUI子系统（无黑框）
# ==============================
add_executable(ClassAssistant WIN32)

target_sources(ClassAssistant PRIVATE ${PROJECT_SOURCES})

# ==============================
# 链接 Qt
# ==============================
target_link_libraries(ClassAssistant PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
)

if(WIN32)
    target_link_libraries(ClassAssistant PRIVATE Qt5::WinExtras)
endif()

# ==============================
# MSVC 编译优化
# ==============================
if(MSVC)
    target_compile_options(ClassAssistant PRIVATE /utf-8)

    # ⭐ 防止新SDK生成Win10子系统（Win7打不开的核心原因）
    set_target_properties(ClassAssistant PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS,6.01"
    )
endif()

# ==============================
# 复制资源
# ==============================
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
