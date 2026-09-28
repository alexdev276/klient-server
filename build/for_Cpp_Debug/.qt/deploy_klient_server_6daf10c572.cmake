include("/Users/sergejzemskov/Институт/Инфокомуникационные сети и системы/Programs/klient-server/build/for_Cpp_Debug/.qt/QtDeploySupport.cmake")
include("${CMAKE_CURRENT_LIST_DIR}/klient-server-plugins.cmake" OPTIONAL)
set(__QT_DEPLOY_I18N_CATALOGS "qtbase")

qt6_deploy_runtime_dependencies(
    EXECUTABLE "klient-server.app"
)
