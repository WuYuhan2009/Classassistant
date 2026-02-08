cmake_minimum_required(VERSION 3.10)

# Qt Creator + Ninja + 新版 CMake 在部分环境下会传入异常的
# CMAKE_CONFIGURATION_TYPES（空值或不含 Debug），导致 project() 阶段
# 的编译器检测 try_compile 报错：No CMake configuration for build type "Debug" found。
# 在 project() 前统一兜底。
if(DEFINED CMAKE_CONFIGURATION_TYPES)
    if(CMAKE_CONFIGURATION_TYPES STREQUAL "")
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "" FORCE)
    else()
        set(_cfg_types ";${CMAKE_CONFIGURATION_TYPES};")
        if(NOT _cfg_types MATCHES ";Debug;")
            set(CMAKE_CONFIGURATION_TYPES "Debug;${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "" FORCE)
        endif()
    endif()
    set(CMAKE_TRY_COMPILE_CONFIGURATION Debug CACHE STRING "" FORCE)
endif()

project(ClassAssistant VERSION 1.0.0 LANGUAGES CXX)

# ==============================
# 基础设置
# ==============================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 单配置生成器默认给 Release，避免空 build type 导致配置异常
if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()


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
