include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_BINARY_DIR}
)

set(knights_SRCS
    core/pos.cpp
    core/move.cpp
    core/item.cpp
    core/piece.cpp

    proto/protocol.cpp
    proto/localprotocol.cpp
    proto/textprotocol.cpp
    proto/ficsprotocol.cpp
    proto/computerprotocol.cpp
    proto/xboardprotocol.cpp
    proto/uciprotocol.cpp
    
    proto/seekgraph.cpp
    proto/chatwidget.cpp
    proto/ficsdialog.cpp
    
    rules/chessrules.cpp

    difficultydialog.cpp
    gamedialog.cpp
    offerwidget.cpp
    clock.cpp
    clockwidget.cpp
    historywidget.cpp
    enginesettings.cpp
    
    externalcontrol.cpp
    gamemanager.cpp
    
    board.cpp
    knightsview.cpp
    knights.cpp
    main.cpp
 )

ki18n_wrap_ui(knights_SRCS
    knightsview_base.ui
    prefs_base.ui
    prefs_access.ui
    gamedialog.ui
    clockwidget.ui
    historywidget.ui
    promotiondialog.ui
    proto/ficsdialog.ui
    proto/chatwidget.ui
    popup.ui
    enginesettings.ui
    customdifficultydialog.ui
)

kconfig_add_kcfg_files(knights_SRCS settings.kcfgc )
qt5_add_dbus_adaptor(knights_SRCS org.kde.Knights.xml externalcontrol.h Knights::ExternalControl)
ecm_qt_declare_logging_category(knights_SRCS HEADER knightsdebug.h IDENTIFIER LOG_KNIGHTS CATEGORY_NAME log_knights)

file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-knights.png")
ecm_add_app_icon(knights_SRCS ICONS ${ICON_SRCS})
add_executable(knights ${knights_SRCS})

if (Qt5TextToSpeech_FOUND)
  target_link_libraries(knights
        Qt5::TextToSpeech)
endif()

target_link_libraries(knights
     Qt5::Network
     Qt5::Svg
     Qt5::Gui
     Qt5::Concurrent
     KF5::Crash
     KF5::Solid
     KF5::WidgetsAddons
     KF5::DBusAddons
     KF5::XmlGui
     KF5KDEGames
     KF5::Plotting
     KF5::Plasma
     KF5::Completion
     KF5::TextWidgets
     KF5::Wallet)

install(TARGETS knights ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS org.kde.knights.desktop  DESTINATION ${KDE_INSTALL_APPDIR} )
install( FILES knights.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES knightsui.rc  DESTINATION  ${KDE_INSTALL_KXMLGUI5DIR}/knights )
install( FILES knights.knsrc  DESTINATION  ${KDE_INSTALL_DATADIR}/knsrcfiles )
install( FILES org.kde.Knights.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )
install( FILES org.kde.knights.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
