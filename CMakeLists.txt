cmake_minimum_required(VERSION 3.16)
project(klient-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)
qt_standard_project_setup()

# Консольное приложение ЛР6 (UDP-чат) и ЛР7 (распределённый поиск ключа Виженера).
# Файлы старой TCP-версии (mainwindow.*, server.*, client.*, адаптеры,
# vigenereclient/vigenereserver/vigenereworker) в сборку не входят.
qt_add_executable(klient-server
    main.cpp
    vigenere.h vigenere.cpp
    workerthread.h workerthread.cpp
    udpserver.h udpserver.cpp
    udpclient.h udpclient.cpp
)

target_link_libraries(klient-server PRIVATE Qt6::Core Qt6::Network)
