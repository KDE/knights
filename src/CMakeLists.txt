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

kde4_add_ui_files(knights_SRCS
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
kde4_add_kcfg_files(knights_SRCS settings.kcfgc )
qt4_add_dbus_interfaces(knights_SRCS ${KDE4_DBUS_INTERFACES_DIR}/org.kde.KSpeech.xml)
qt4_add_dbus_adaptor(knights_SRCS org.kde.Knights.xml externalcontrol.h Knights::ExternalControl)
kde4_add_executable(knights ${knights_SRCS})

target_link_libraries(knights ${KDE4_KDEUI_LIBS} ${KDE4_KPARTS_LIBS} ${KDE4_PLASMA_LIBS} ${KDE4_SOLID_LIBS} ${KDEGAMES_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS knights ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS knights.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES knights.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES knightsui.rc  DESTINATION  ${DATA_INSTALL_DIR}/knights )
install( FILES knights.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )
install( FILES org.kde.Knights.xml DESTINATION ${DBUS_INTERFACES_INSTALL_DIR} )


