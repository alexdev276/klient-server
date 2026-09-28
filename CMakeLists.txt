cmake_minimum_required(VERSION 3.19)
project(klient-server LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()

qt_add_executable(klient-server
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    isocketadapter.h
    socketadapter.h socketadapter.cpp
    serversocketadapter.h serversocketadapter.cpp
    server.h server.cpp
)

target_link_libraries(klient-server
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS klient-server
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET klient-server
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
