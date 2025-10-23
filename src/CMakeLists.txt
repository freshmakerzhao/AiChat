cmake_minimum_required(VERSION 3.14)

project(qt5_component_AI_chat)

# Set C++17 as required standard for the project
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    # 引入配置文件
    include(${CMAKE_SOURCE_DIR}/config_scripts/config.cmake)

    message(STATUS "VCPKG_PATH set to: ${VCPKG_PATH}")
    message(STATUS "RESOURCE_WIN_PATH set to: ${RESOURCE_WIN_PATH}")
    message(STATUS "FONTS_PATH set to: ${FONTS_PATH}")

    set(CMAKE_TOOLCHAIN_FILE "${VCPKG_PATH}/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")
    set(CMAKE_PREFIX_PATH "${VCPKG_PATH}/installed/x64-windows;${CMAKE_PREFIX_PATH}")
endif()

# 检测操作系统
if(WIN32)
    message(STATUS "Configuring on/for Windows")
    # 特定于Windows的编译选项
    add_definitions(-DWINDOWS_PLATFORM)
    # Copy qt.conf
    configure_file(${CMAKE_SOURCE_DIR}/qt.conf ${CMAKE_BINARY_DIR}/qt.conf COPYONLY)
elseif(UNIX AND NOT APPLE)
    message(STATUS "Configuring on/for Linux")
    # 特定于Linux的编译选项
    add_definitions(-DLINUX_PLATFORM)
    set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
elseif(APPLE)
    message(STATUS "Configuring on/for macOS")
    # 特定于macOS的编译选项
    add_definitions(-DMACOS_PLATFORM)
endif()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt5 COMPONENTS PrintSupport)
find_package(Qt5 COMPONENTS OpenGL)
find_package(Qt5 COMPONENTS Core Network REQUIRED)
find_package(Qt5 COMPONENTS Core Xml REQUIRED)
find_package(md4c CONFIG REQUIRED)

# Add source files
file(GLOB PROJECT_SOURCES
    utils/*.cpp
    widgets/*.cpp
    core/*.cpp
    network/*.cpp
    *.cpp
)

# Add header files
file(GLOB PROJECT_HEADERS
    utils/*.h
    widgets/*.h
    core/*.h
    network/*.h
    *.h
)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add resource files
set(PROJECT_RESOURCES
    res.qrc
)

# release模式下不弹出控制台
if(MSVC)
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_executable(${PROJECT_NAME} WIN32
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${PROJECT_RESOURCES}
        )
    else()
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${PROJECT_RESOURCES}
        )
    endif()
elseif(MINGW)
        add_executable(${PROJECT_NAME}
            ${PROJECT_SOURCES}
            ${PROJECT_HEADERS}
            ${PROJECT_RESOURCES}
            win32.rc
        )
        if(CMAKE_BUILD_TYPE STREQUAL "Release")
            set(CMAKE_CXX_FLAGS "-mwindows")
        endif()
endif()


# Link the Qt5 Widgets library and Third-Party library to the executable
target_link_libraries(${PROJECT_NAME}
    Qt5::Widgets
    Qt5::Core
    Qt5::Network
    Qt5::Xml
    Qt5::PrintSupport
    Qt5::OpenGL
    psapi
    oleaut32
    uuid
    md4c::md4c
    md4c::md4c-html
)

if (Qt5PrintSupport_FOUND)
    target_link_libraries(${PROJECT_NAME} Qt5::PrintSupport)
endif()

if (Qt5OpenGL_FOUND)
    target_link_libraries(${PROJECT_NAME} Qt5::OpenGL)
endif()


if(MSVC)
    # === 加载 Qt DLL 拷贝脚本 ===
    include(${CMAKE_SOURCE_DIR}/config_scripts/deploy_qt_dependencies.cmake)
    my_deploy_dependencies(${PROJECT_NAME})
endif()
message(STATUS "✅ Configuration finished for ${PROJECT_NAME}")
