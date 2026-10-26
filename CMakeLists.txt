cmake_minimum_required(VERSION 3.16)
project(klient-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Network Widgets)

qt_standard_project_setup()

qt_add_executable(klient-server
    main.cpp
    isocketadapter.h
    socketadapter.h socketadapter.cpp
    serversocketadapter.h serversocketadapter.cpp
    clientsocketadapter.h clientsocketadapter.cpp
    udpadapter.h udpadapter.cpp
    server.h server.cpp
    client.h client.cpp
    serversettingsdialog.h serversettingsdialog.cpp
    udpserver.h udpserver.cpp
    udpclient.h udpclient.cpp
    vigenereworker.h vigenereworker.cpp
    vigenereserver.h vigenereserver.cpp
    vigenereclient.h vigenereclient.cpp
)

target_link_libraries(klient-server PRIVATE
    Qt6::Core Qt6::Gui Qt6::Network Qt6::Widgets
)