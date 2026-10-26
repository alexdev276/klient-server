cmake_minimum_required(VERSION 3.16)
project(klient-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup()

# В сборку включены только файлы для UDP версии (ЛР4-5)
qt_add_executable(klient-server
    main.cpp
    isocketadapter.h
    socketadapter.h
    socketadapter.cpp
    clientsocketadapter.h
    clientsocketadapter.cpp
    serversocketadapter.h
    serversocketadapter.cpp
    server.h
    server.cpp
    client.h
    client.cpp

    # --- СТАРЫЕ ФАЙЛЫ (ЗАКОММЕНТИРОВАНЫ, ЧТОБЫ НЕ УДАЛЯТЬ ИХ С ДИСКА) ---
    # vigenereserver.h
    # vigenereserver.cpp
    # vigenereclient.h
    # vigenereclient.cpp
    # vigenereworker.h
    # vigenereworker.cpp
    # udpserver.h
    # udpserver.cpp
    # udpclient.h
    # udpclient.cpp
)

target_link_libraries(klient-server PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)