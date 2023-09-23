ecm_setup_version(${KNIGHTS_VERSION}
    VARIABLE_PREFIX KNIGHTS
    VERSION_HEADER knights_version.h
)

add_executable(knights)

set(dbus_SRCS)
qt_add_dbus_adaptor(dbus_SRCS org.kde.Knights.xml externalcontrol.h Knights::ExternalControl)

target_sources(knights PRIVATE
    ${dbus_SRCS}

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
    knights.qrc
 )

ki18n_wrap_ui(knights
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

kconfig_add_kcfg_files(knights settings.kcfgc )

ecm_qt_declare_logging_category(knights
    HEADER knightsdebug.h
    IDENTIFIER LOG_KNIGHTS
    CATEGORY_NAME log_knights
    DESCRIPTION "knights"
    EXPORT KNIGHTS
)

file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-knights.png")
ecm_add_app_icon(knights ICONS ${ICON_SRCS})

if (Qt6TextToSpeech_FOUND)
     target_link_libraries(knights
         Qt6::TextToSpeech
     )
endif()

target_link_libraries(knights
     KDEGames6
     KF6::CoreAddons
     KF6::Crash
     KF6::Solid
     KF6::WidgetsAddons
     KF6::DBusAddons
     KF6::XmlGui
     KF6::Plotting
     KF6::Completion
     KF6::Svg
     KF6::TextWidgets
     KF6::Wallet
     Qt6::Network
     Qt6::Svg
     Qt6::Gui
     Qt6::Concurrent
     Qt6::Core5Compat
)

install(TARGETS knights ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

install( PROGRAMS org.kde.knights.desktop  DESTINATION ${KDE_INSTALL_APPDIR} )
install( FILES knights.kcfg  DESTINATION  ${KDE_INSTALL_KCFGDIR} )
install( FILES knights.knsrc  DESTINATION  ${KDE_INSTALL_DATADIR}/knsrcfiles )
install( FILES org.kde.Knights.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
install( FILES org.kde.knights.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
