set(knights_SRCS
    core/pos.cpp
    core/piece.cpp
    core/move.cpp
    proto/protocol.cpp
    proto/xboardproto.cpp
 #   proto/ficsprotocol.cpp
    chessrules.cpp
    gamedialog.cpp
    board.cpp
    clockwidget.cpp
    knightsview.cpp
    knights.cpp
    main.cpp
 )

kde4_add_ui_files(knights_SRCS knightsview_base.ui prefs_base.ui gamedialog.ui clockwidget.ui)
kde4_add_kcfg_files(knights_SRCS settings.kcfgc )
kde4_add_executable(knights ${knights_SRCS})

target_link_libraries(knights ${KDE4_KDEUI_LIBS} kdegames)

install(TARGETS knights ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES knights.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES knights.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES knightsui.rc  DESTINATION  ${DATA_INSTALL_DIR}/knights )
