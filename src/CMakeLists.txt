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

if (${KDE_VERSION} VERSION_GREATER "4.5.60")
    message("Building with KGameRenderer support")
    add_definitions(-DHAVE_RENDERER)
endif (${KDE_VERSION} VERSION_GREATER "4.5.60")

kde4_add_ui_files(knights_SRCS knightsview_base.ui prefs_base.ui gamedialog.ui clockwidget.ui proto/ficsdialog.ui)
kde4_add_kcfg_files(knights_SRCS settings.kcfgc )
kde4_add_executable(knights ${knights_SRCS})

target_link_libraries(knights ${KDE4_KDEUI_LIBS} kdegames ${QT_QTNETWORK_LIBRARY})

install(TARGETS knights ${INSTALL_TARGETS_DEFAULT_ARGS} )

########### install files ###############

install( FILES knights.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES knights.kcfg  DESTINATION  ${KCFG_INSTALL_DIR} )
install( FILES knightsui.rc  DESTINATION  ${DATA_INSTALL_DIR}/knights )
