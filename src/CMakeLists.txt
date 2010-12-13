set(knights_SRCS
    core/pos.cpp
    core/move.cpp
    core/renderer.cpp
    core/item.cpp
    core/piece.cpp
    proto/protocol.cpp
    proto/xboardproto.cpp
    proto/ficsprotocol.cpp
    proto/ficsdialog.cpp
    rules/chessrules.cpp
    gamedialog.cpp
    board.cpp
    clock.cpp
    clockwidget.cpp
    knightsview.cpp
    knights.cpp
    main.cpp
 )

if (KGR)
    add_definitions(-DWITH_KGR)
endif (KGR)

if (${QT_VERSION} VERSION_GREATER "4.6.0")
    add_definitions(-DWITH_QT_46)
endif()

if (ANIMATIONS)
    add_definitions(-DWITH_ANIMATIONS)
endif()

kde4_add_ui_files(knights_SRCS knightsview_base.ui prefs_base.ui gamedialog.ui clockwidget.ui proto/ficsdialog.ui promotiondialog.ui)
kde4_add_kcfg_files(knights_SRCS settings.kcfgc )
kde4_add_executable(knights ${knights_SRCS})

target_link_libraries(knights ${KDE4_KDEUI_LIBS} ${KDE4_PLASMA_LIBS} ${KDEGAMES_LIBRARY} ${QT_QTNETWORK_LIBRARY})

install(TARGETS knights ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( PROGRAMS knights.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES knights.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES knightsui.rc  DESTINATION  ${DATA_INSTALL_DIR}/knights )
install( FILES knights.knsrc  DESTINATION  ${CONFIG_INSTALL_DIR} )

