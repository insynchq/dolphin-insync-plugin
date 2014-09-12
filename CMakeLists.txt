project(dolphin-insync-plugin)

FIND_PACKAGE( KDE4 REQUIRED )
FIND_PACKAGE(LibKonq REQUIRED)
INCLUDE( KDE4Defaults )

include_directories( ${QT_INCLUDES} ${LIBKONQ_INCLUDE_DIR} )

set(dolphin-insync-plugin_SRCS dolphin-insync-plugin.cpp)
kde4_add_plugin(dolphin-insync-plugin  ${dolphin-insync-plugin_SRCS})
target_link_libraries(dolphin-insync-plugin ${KDE4_KIO_LIBS} ${LIBKONQ_LIBRARY} qjson)

install(FILES dolphin-insync-plugin.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS dolphin-insync-plugin DESTINATION ${PLUGIN_INSTALL_DIR})
