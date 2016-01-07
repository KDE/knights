file(GLOB ICONS_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/*-apps-knights.png")
ecm_add_app_icon(knights_SRCS ICONS ${ICONS_SRCS})

ecm_install_icons(ICONS ${ICONS_SRCS} DESTINATION ${KDE_INSTALL_ICONDIR} THEME hicolor)
