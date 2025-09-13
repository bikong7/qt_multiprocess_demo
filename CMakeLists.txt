cmake_minimum_required(VERSION 3.16)
project(MultiProcessDemo VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# 启用自动处理 MOC、UIC、RCC
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

if (WIN32)
    # 强制使用控制台子系统，这样 qDebug() 输出到控制台
    set(CMAKE_WIN32_EXECUTABLE OFF)
endif()

# 添加源文件
set(SOURCES
    main.cpp
    dataview/mainwindow.cpp
    storage/storage.cpp
    common/sharedmemory_manager.cpp
)

set(HEADERS
    dataview/mainwindow.h
    storage/storage.h
    common/sharedmemory_manager.h
    common/common.h
)

# 添加可执行文件
add_executable(MyApp1
    ${SOURCES}
    ${HEADERS}
)

# 链接 Qt5 库
target_link_libraries(MyApp1 PRIVATE Qt5::Core Qt5::Widgets)
