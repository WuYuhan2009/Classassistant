cmake_minimum_required(VERSION 3.10)

# Qt Creator + Ninja + CMake 4.x + MSVC 组合下的稳定性兜底：
# 1) 补齐 Debug 构建类型，避免 "No CMake configuration for build type \"Debug\" found"
# 2) 固定 try_compile 走 Debug + STATIC_LIBRARY，减少探测阶段差异
# 3) 提前设置 MSVC 调试信息为 /Z7，降低 PDB 争用导致的 C1041
if(NOT DEFINED CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

if(DEFINED CMAKE_CONFIGURATION_TYPES)
    if(CMAKE_CONFIGURATION_TYPES STREQUAL "")
        set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo;MinSizeRel" CACHE STRING "" FORCE)
    else()
        set(_cfg_types ";${CMAKE_CONFIGURATION_TYPES};")
        if(NOT _cfg_types MATCHES ";Debug;")
            set(CMAKE_CONFIGURATION_TYPES "Debug;${CMAKE_CONFIGURATION_TYPES}" CACHE STRING "" FORCE)
        endif()
    endif()
endif()

set(CMAKE_TRY_COMPILE_CONFIGURATION Debug CACHE STRING "" FORCE)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY CACHE STRING "" FORCE)

if(MSVC)
    if(POLICY CMP0141)
        cmake_policy(SET CMP0141 NEW)
    endif()
    set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT
        "$<$<CONFIG:Debug,RelWithDebInfo>:Embedded>"
        CACHE STRING "" FORCE)
    set(CMAKE_C_FLAGS_DEBUG "/MDd /Ob0 /Od /RTC1 /FS /Z7" CACHE STRING "" FORCE)
    set(CMAKE_CXX_FLAGS_DEBUG "/MDd /Ob0 /Od /RTC1 /FS /Z7" CACHE STRING "" FORCE)
endif()

project(ClassFlow VERSION 2.0.1 LANGUAGES CXX)

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
find_package(Qt5 COMPONENTS Core Gui Widgets Network REQUIRED)

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
    src/ui/FluentTheme.h
    src/ui/FluentTheme.cpp
    src/ui/Tools.h
    src/ui/Tools.cpp
    resources.qrc
)

# ==============================
# 创建可执行程序
# WIN32 = GUI子系统（无黑框）
# ==============================
add_executable(ClassFlow WIN32)

target_sources(ClassFlow PRIVATE ${PROJECT_SOURCES})

# ==============================
# 链接 Qt
# ==============================
target_link_libraries(ClassFlow PRIVATE
    Qt5::Core
    Qt5::Gui
    Qt5::Widgets
    Qt5::Network
)

if(WIN32)
    target_link_libraries(ClassFlow PRIVATE Qt5::WinExtras)
endif()

# ==============================
# MSVC 编译优化
# ==============================
if(MSVC)
    target_compile_options(ClassFlow PRIVATE /utf-8)

    # ⭐ 防止新SDK生成Win10子系统（Win7打不开的核心原因）
    set_target_properties(ClassFlow PROPERTIES
        LINK_FLAGS "/SUBSYSTEM:WINDOWS,6.01"
    )
endif()

# ==============================
# 复制资源
# ==============================
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/docs DESTINATION ${CMAKE_BINARY_DIR})
