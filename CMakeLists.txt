cmake_minimum_required(VERSION 3.16)

project(insyncdolphinplugins LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include(FeatureSummary)
include(CMakePackageConfigHelpers)

# Find required packages
find_package(Qt6 CONFIG REQUIRED COMPONENTS Core Network Widgets)
find_package(ECM REQUIRED NO_MODULE)
find_package(KF6Config REQUIRED)
find_package(KF6CoreAddons REQUIRED)
find_package(KF6KIO REQUIRED)
find_package(KF6WidgetsAddons REQUIRED)

# Set up CMake module path
set(CMAKE_MODULE_PATH ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include_directories(${CMAKE_CURRENT_BINARY_DIR})

# Include KDE-specific CMake modules
include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(ECMMarkNonGuiExecutable)
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(ECMQtDeclareLoggingCategory)
include(GenerateExportHeader)


# Set package properties
set_package_properties(insyncdolphinplugins PROPERTIES
    DESCRIPTION "The Insync Dolphin plugin that adds Insync's context menus and overlay icons"
    URL "https://www.insynchq.com"
    TYPE REQUIRED
    PURPOSE "Provides Insync's implementation of the file item action plugin interface."
)

# Add definitions and set RPATH
add_definitions(-DQT_NO_URL_CAST_FROM_STRING -DQT_NO_FOREACH -DQT_NO_KEYWORDS)
set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)
set(CMAKE_INSTALL_RPATH "")

# Dolphin plugin helper
add_definitions(-DTRANSLATION_DOMAIN=\"insyncdolphinpluginhelper\")
set(INSYNC_DOLPHIN_PLUGIN_HELPER insyncdolphinpluginhelper)
add_library(${INSYNC_DOLPHIN_PLUGIN_HELPER} SHARED insyncdolphinpluginhelper.cpp
    insyncdolphinpluginhelper.hpp)
target_link_libraries(${INSYNC_DOLPHIN_PLUGIN_HELPER} PRIVATE Qt6::Network)
generate_export_header(${INSYNC_DOLPHIN_PLUGIN_HELPER} BASE_NAME insyncdolphinpluginhelper)
install(TARGETS ${INSYNC_DOLPHIN_PLUGIN_HELPER} LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Item action plugin
add_definitions(-DTRANSLATION_DOMAIN=\"insyncfileitemaction\")
set(INSYNC_ACTION_PLUGIN insyncfileitemaction)
add_library(${INSYNC_ACTION_PLUGIN} MODULE ${INSYNC_ACTION_PLUGIN}.cpp
    insyncfileitemaction.hpp)
target_link_libraries(${INSYNC_ACTION_PLUGIN} PRIVATE
    KF6::CoreAddons
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    ${INSYNC_DOLPHIN_PLUGIN_HELPER}
)
install(TARGETS ${INSYNC_ACTION_PLUGIN} DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/kfileitemaction)

# Overlay icon plugin
add_definitions(-DTRANSLATION_DOMAIN=\"insyncoverlayicon\")
set(INSYNC_OVERLAY_PLUGIN insyncoverlayicon)
add_library(${INSYNC_OVERLAY_PLUGIN} MODULE ${INSYNC_OVERLAY_PLUGIN}.cpp
    insyncoverlayicon.hpp
    insyncoverlayicon.json)
target_link_libraries(${INSYNC_OVERLAY_PLUGIN} PRIVATE
    KF6::CoreAddons
    KF6::KIOCore
    KF6::KIOFileWidgets
    KF6::KIOWidgets
    ${INSYNC_DOLPHIN_PLUGIN_HELPER}
)
install(TARGETS ${INSYNC_OVERLAY_PLUGIN} DESTINATION ${KDE_INSTALL_PLUGINDIR}/kf6/overlayicon)

# Icons
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/usr/share/icons/
    DESTINATION ${KDE_INSTALL_ICONDIR}
    FILES_MATCHING
    PATTERN "*.png"
    PATTERN "*.svg"
)

# Remove files from previous installations
# They were renamed and if they are still present, the overlay icons appear twice
# Luckily when the files are not there, this fails silently
install(CODE "
    file(REMOVE 
		\"\${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_PLUGINDIR}/kf6/overlayicon/overlayiconinsyncplugin.so\"
        \"\${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_PLUGINDIR}/kf6/kfileitemaction/fileitemactioninsyncplugin.so\"
        \"\${CMAKE_INSTALL_PREFIX}/${KDE_INSTALL_DATADIR}/kio/servicemenus/fileitemactioninsyncplugin.desktop\"
    )
")

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)
