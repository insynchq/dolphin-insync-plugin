project(insyncdolphinplugins)
cmake_minimum_required(VERSION 2.8.12 FATAL_ERROR)

include(FeatureSummary)

set(QT_MIN_VERSION "5.5.1")
set(KF5_MIN_VERSION "5.16.0")

find_package(Qt5 ${QT_MIN_VERSION} CONFIG REQUIRED COMPONENTS Core Network)
find_package(ECM ${KF5_MIN_VERSION} REQUIRED NO_MODULE)

set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS KIO)

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings)
include(ECMMarkNonGuiExecutable)
include(GenerateExportHeader)

set_package_properties(insyncdolphinplugins PROPERTIES
    DESCRIPTION "The Insync Dolphin plugin that adds Insync's context menus and overlay icons"
    URL "https://www.insynchq.com"
    TYPE REQUIRED
    PURPOSE "Provides Insync's implementation of the file item action plugin interface."
)

add_definitions(-DQT_NO_URL_CAST_FROM_STRING -DQT_NO_FOREACH -DQT_NO_KEYWORDS)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "")

# Dolphin plugin helper
set(INSYNC_DOLPHIN_PLUGIN_HELPER insyncdolphinpluginhelper)
add_library(${INSYNC_DOLPHIN_PLUGIN_HELPER} SHARED insyncdolphinpluginhelper.cpp)
target_link_libraries(${INSYNC_DOLPHIN_PLUGIN_HELPER} Qt5::Network)
generate_export_header(${INSYNC_DOLPHIN_PLUGIN_HELPER} BASE_NAME insyncdolphinpluginhelper)
install(TARGETS ${INSYNC_DOLPHIN_PLUGIN_HELPER} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Item action plugin
set(INSYNC_ACTION_PLUGIN fileitemactioninsyncplugin)
add_library(${INSYNC_ACTION_PLUGIN} MODULE ${INSYNC_ACTION_PLUGIN}.cpp)
target_link_libraries(${INSYNC_ACTION_PLUGIN}
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    ${INSYNC_DOLPHIN_PLUGIN_HELPER}
)
install(FILES ${INSYNC_ACTION_PLUGIN}.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS ${INSYNC_ACTION_PLUGIN} DESTINATION ${PLUGIN_INSTALL_DIR})

# Overlay icon plugin
set(INSYNC_OVERLAY_PLUGIN overlayiconinsyncplugin)
add_library(${INSYNC_OVERLAY_PLUGIN} MODULE ${INSYNC_OVERLAY_PLUGIN}.cpp)
target_link_libraries(${INSYNC_OVERLAY_PLUGIN}
    KF5::CoreAddons
    KF5::KIOCore
    KF5::KIOWidgets
    ${INSYNC_DOLPHIN_PLUGIN_HELPER}
)
install(TARGETS ${INSYNC_OVERLAY_PLUGIN} DESTINATION ${PLUGIN_INSTALL_DIR}/kf5/overlayicon)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
