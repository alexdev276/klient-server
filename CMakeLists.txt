cmake_minimum_required(VERSION 3.16)

project(klient-server VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Добавляем модуль Network
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    # Серверные файлы по методичке
    isocketadapter.h
    socketadapter.h
    socketadapter.cpp
    serversocketadapter.h
    serversocketadapter.cpp
    server.h
    server.cpp
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(klient-server
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        README.md
        clientsocketadapter.h clientsocketadapter.cpp
        client.h client.cpp
    )
else()
    add_executable(klient-server ${PROJECT_SOURCES})
endif()

# Подключаем модуль Network к проекту
target_link_libraries(klient-server PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)

set_target_properties(klient-server PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(klient-server)
endif()