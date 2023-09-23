ecm_setup_version(${KNIGHTS_VERSION}
    VARIABLE_PREFIX KNIGHTS
    VERSION_HEADER knights_version.h
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
    knights.qrc
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
qt_add_dbus_adaptor(knights_SRCS org.kde.Knights.xml externalcontrol.h Knights::ExternalControl)
ecm_qt_declare_logging_category(knights_SRCS HEADER knightsdebug.h IDENTIFIER LOG_KNIGHTS CATEGORY_NAME log_knights DESCRIPTION "knights" EXPORT KNIGHTS)

file(GLOB ICON_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../icons/*-apps-knights.png")
ecm_add_app_icon(knights_SRCS ICONS ${ICON_SRCS})
add_executable(knights ${knights_SRCS})

if (Qt${QT_MAJOR_VERSION}TextToSpeech_FOUND)
  target_link_libraries(knights
        Qt${QT_MAJOR_VERSION}::TextToSpeech)
endif()

target_link_libraries(knights
     Qt${QT_MAJOR_VERSION}::Network
     Qt${QT_MAJOR_VERSION}::Svg
     Qt${QT_MAJOR_VERSION}::Gui
     Qt${QT_MAJOR_VERSION}::Concurrent
     KF${KF_MAJOR_VERSION}::CoreAddons
     KF${KF_MAJOR_VERSION}::Crash
     KF${KF_MAJOR_VERSION}::Solid
     KF${KF_MAJOR_VERSION}::WidgetsAddons
     KF${KF_MAJOR_VERSION}::DBusAddons
     KF${KF_MAJOR_VERSION}::XmlGui
     KF${KF_MAJOR_VERSION}::Plotting
     KF${KF_MAJOR_VERSION}::Completion
     KF${KF_MAJOR_VERSION}::TextWidgets
     KF${KF_MAJOR_VERSION}::Wallet
)

if (QT_MAJOR_VERSION STREQUAL "6")
    target_link_libraries(knights
        Qt6::Core5Compat
        KDEGames6
        KF6::Svg
    )
else()
    target_link_libraries(knights
        KF5KDEGames
        KF5::Plasma
    )
endif()

install(TARGETS knights ${KDE_INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS org.kde.knights.desktop  DESTINATION ${KDE_INSTALL_APPDIR} )
install( FILES knights.kcfg  DESTINATION  ${KDE_INSTALL_KCFGDIR} )
install( FILES knights.knsrc  DESTINATION  ${KDE_INSTALL_DATADIR}/knsrcfiles )
install( FILES org.kde.Knights.xml DESTINATION ${KDE_INSTALL_DBUSINTERFACEDIR} )
install( FILES org.kde.knights.appdata.xml DESTINATION ${KDE_INSTALL_METAINFODIR} )
