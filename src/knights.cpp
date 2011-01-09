/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "knights.h"

#include "core/piece.h"
#include "proto/xboardproto.h"
#include "proto/ficsprotocol.h"
#include "knightsview.h"
#include "settings.h"
#include "gamedialog.h"
#include "clockwidget.h"

#include <KConfigDialog>
#include <KStatusBar>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KToggleAction>
#include <KGameThemeSelector>
#include <KStandardGameAction>
#include <KLocale>
#include <KMessageBox>
#include <KMenuBar>
#include <KMenu>

#include <QtGui/QDropEvent>
#include <QtCore/QTimer>
#include <QtGui/QDockWidget>
#include "proto/localprotocol.h"
#include <KUser>

namespace Knights
{
    MainWindow::MainWindow()
            : KXmlGuiWindow(),
            m_view ( new KnightsView ( this ) ),
            m_clockDock ( 0 )
    {
        // accept dnd
        setAcceptDrops ( true );

        // tell the KXmlGuiWindow that this is indeed the main widget
        setCentralWidget ( m_view );

        // then, setup our actions
        setupActions();

        // add a status bar
        statusBar()->show();

        // a call to KXmlGuiWindow::setupGUI() populates the GUI
        // with actions, using KXMLGUI.
        // It also applies the saved mainwindow settings, if any, and ask the
        // mainwindow to automatically save settings if changed: window size,
        // toolbar position, icon size, etc.
        setupGUI();
        connect ( m_view, SIGNAL ( gameNew() ), this, SLOT ( fileNew() ) );

        QTimer::singleShot ( 0, this, SLOT ( fileNew() ) );
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::setupActions()
    {
        KStandardGameAction::gameNew ( this, SLOT ( fileNew() ), actionCollection() );
        KStandardGameAction::quit ( qApp, SLOT ( closeAllWindows() ), actionCollection() );
        KStandardAction::preferences ( this, SLOT ( optionsPreferences() ), actionCollection() );
    }

    void MainWindow::fileNew()
    {
        KDialog gameNewDialog;
        GameDialog* dialogWidget = new GameDialog ( &gameNewDialog );
        gameNewDialog.setMainWidget ( dialogWidget );
        gameNewDialog.setCaption ( i18n ( "New Game" ) );
        if ( gameNewDialog.exec() == KDialog::Accepted )
        {
            delete m_protocol;
            foreach ( QDockWidget* dock, m_dockWidgets )
            {
                removeDockWidget ( dock );
                delete dock;
            }
            m_dockWidgets.clear();
            // Remove protocol-dependent actions
            foreach ( QAction* action, m_protocolActions )
            {
                actionCollection()->removeAction(action);
            }
            m_protocolActions.clear();

            m_view->clearBoard();
            dialogWidget->writeConfig();

            QVariantMap protocolOptions;
            switch ( dialogWidget->protocol() )
            {
                case Settings::EnumProtocol::XBoard:
                    m_protocol = new XBoardProtocol;
                    protocolOptions[QLatin1String ( "program" ) ] = dialogWidget->program();
                    protocolOptions[QLatin1String ( "PlayerName" ) ] = KUser().fullName();
                    break;
                case Settings::EnumProtocol::FICS:
                    m_protocol = new FicsProtocol;
                    protocolOptions[QLatin1String ( "server" ) ] = dialogWidget->server();
                    break;
                default:
                    m_protocol = new LocalProtocol;
                    break;
            }

            m_timeLimit = dialogWidget->timeLimit();
            if ( m_timeLimit )
            {
                m_protocol->setTimeControl (
                                            dialogWidget->color(),
                                            dialogWidget->playerMoves(),
                                            dialogWidget->playerTime().hour() * 60 + dialogWidget->playerTime().minute(),
                                            dialogWidget->playerIncrement()
                );
                m_protocol->setTimeControl (
                                            oppositeColor ( dialogWidget->color() ),
                                            dialogWidget->opponentMoves(),
                                            dialogWidget->opponentTime().hour() * 60 + dialogWidget->opponentTime().minute(),
                                            dialogWidget->opponentIncrement()
                );
            }
                protocolOptions[QLatin1String ( "PlayerColors" ) ] = QVariant::fromValue<Colors> ( dialogWidget->color() );
                connect ( m_protocol, SIGNAL ( initSuccesful() ), SLOT ( protocolInitSuccesful() ), Qt::QueuedConnection );
                connect ( m_protocol, SIGNAL ( error ( Protocol::ErrorCode, QString ) ), SLOT ( protocolError ( Protocol::ErrorCode, QString ) ), Qt::QueuedConnection );
                m_protocol->init ( protocolOptions );
        }
    }

    void MainWindow::undo()
    {
        QAction* pauseAction = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));
        if ( pauseAction && !pauseAction->isChecked() )
        {
            pauseAction->trigger();
        }
        if ( m_protocol )
        {
            m_protocol->undoLastMove();
        }
    }

    void MainWindow::redo()
    {
        QAction* pauseAction = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));

        if ( pauseAction && !pauseAction->isChecked() )
        {
            pauseAction->trigger();
        }
        
        if ( m_protocol )
        {
            m_protocol->redoLastMove();
        }
    }

    void MainWindow::protocolInitSuccesful()
    {
        m_view->setProtocol ( m_protocol );
        QString whiteName = m_protocol->playerName();
        QString blackName = m_protocol->opponentName();
        if ( !( m_protocol->playerColors() & White ) )
        {
            qSwap ( whiteName, blackName );
        }
        setCaption( i18n ( "%1 vs. %2", whiteName, blackName ) );
        
            Protocol::Features f = m_protocol->supportedFeatures();

            if ( f & Protocol::SetTimeLimit )
            {
                // The time limit was set or changed by the protocol
                m_timeLimit = m_protocol->attribute ( QLatin1String ( "TimeLimitEnabled" ) ).toBool();
                if ( m_timeLimit )
                {
                    m_playerTime = m_protocol->attribute ( QLatin1String ( "playerTime" ) ).toTime();
                    m_oppTime = m_protocol->attribute ( QLatin1String ( "opponentTime" ) ).toTime();
                }
            }
            
            if ( f & Protocol::Draw )
            {
                KAction* drawAction = actionCollection()->addAction ( QLatin1String ( "propose_draw" ), m_protocol, SLOT ( proposeDraw() ) );
                drawAction->setText ( i18n ( "Propose &Draw" ) );
                drawAction->setHelpText(i18n("Propose a draw to your opponent"));
                drawAction->setIcon(KIcon(QLatin1String("flag-blue")));
                m_protocolActions << drawAction;
            }
            if ( f & Protocol::Resign )
            {
                KAction* resignAction = actionCollection()->addAction ( QLatin1String ( "resign" ), m_protocol, SLOT ( resign() ) );
                resignAction->setText ( i18n ( "Resign" ) );
                resignAction->setHelpText(i18n("Admit your inevitable defeat"));
                resignAction->setIcon(KIcon(QLatin1String("flag-red")));
                m_protocolActions << resignAction;
            }
            if ( f & Protocol::Adjourn )
            {
                KAction* adjournAction = actionCollection()->addAction ( QLatin1String ( "adjourn" ), m_protocol, SLOT ( adjourn() ) );
                adjournAction->setText ( i18n ( "Adjourn" ) );
                adjournAction->setHelpText(i18n("Continue this game at a later time"));
                adjournAction->setIcon(KIcon(QLatin1String("document-save")));
                m_protocolActions << adjournAction;
            }
            if ( f & Protocol::Pause )
            {
                m_protocolActions << KStandardGameAction::pause ( this, SLOT ( pauseGame ( bool ) ), actionCollection() );
            }
            if ( f & Protocol::Undo )
            {
                KAction* undoAction = KStandardAction::undo( this, SLOT(undo()), actionCollection() );
                undoAction->setEnabled(false);
                connect ( m_protocol, SIGNAL(undoPossible(bool)), undoAction, SLOT(setEnabled(bool)) );
                m_protocolActions << undoAction;
                
                KAction* redoAction = KStandardAction::redo( this, SLOT(redo()), actionCollection() );
                redoAction->setEnabled(false);
                connect ( m_protocol, SIGNAL(redoPossible(bool)), redoAction, SLOT(setEnabled(bool)) );
                m_protocolActions << redoAction;
            }
            foreach ( const Protocol::ToolWidgetData& data, m_protocol->toolWidgets() )
            {
                QDockWidget* dock = new QDockWidget ( data.title, this );
                dock->setWidget ( data.widget );
                dock->setObjectName ( data.name + QLatin1String("DockWidget") );
                QAction* dockToggleAction = dock->toggleViewAction();
                actionCollection()->addAction ( data.name, dockToggleAction );
                m_protocolActions << dockToggleAction;
                m_dockWidgets << dock;
                addDockWidget ( Qt::LeftDockWidgetArea, dock );
            }
            if ( m_timeLimit )
        {
            showClockWidgets();
        }
            createGUI();

        QTimer::singleShot(1, m_view, SLOT(setupBoard()));
        
}

    void MainWindow::showClockWidgets()
    {
        ClockWidget* playerClock = new ClockWidget ( this );
        m_clockDock = new QDockWidget ( i18n ( "Clock" ), this );
        m_clockDock->setObjectName ( QLatin1String ( "ClockDockWidget" ) ); // for QMainWindow::saveState()
        m_clockDock->setWidget ( playerClock );
        QAction* clockToggleAction = m_clockDock->toggleViewAction();
        actionCollection()->addAction ( QLatin1String("clock"), clockToggleAction );
        m_protocolActions << clockToggleAction;
        m_dockWidgets << m_clockDock;
        
        connect ( m_view, SIGNAL ( displayedPlayerChanged ( Color ) ), playerClock, SLOT ( setDisplayedPlayer ( Color ) ) );

        Colors playerColors = m_protocol->playerColors();
        if ( playerColors == White || playerColors == Black )
        {
            Color playerColor = ( playerColors & White ) ? White : Black;
            if ( !m_protocol->playerName().isEmpty() )
            {
                playerClock->setPlayerName ( playerColor, m_protocol->playerName() );
            }
            else
            {
                playerClock->setPlayerName ( playerColor, i18n ( "You" ) );
            }
            if ( !m_protocol->opponentName().isEmpty() )
            {
                playerClock->setPlayerName ( oppositeColor ( playerColor ), m_protocol->opponentName() );
            }
            else
            {
                playerClock->setPlayerName ( oppositeColor ( playerColor ), i18n ( "Opponent" ) );
            }
            if ( m_protocol->supportedFeatures() & Protocol::UpdateTime )
            {
                connect ( m_protocol, SIGNAL(timeChanged(Color,QTime)), playerClock, SLOT(setCurrentTime(Color,QTime)) );
                connect ( m_protocol, SIGNAL(timeLimitChanged(Color,QTime)), playerClock, SLOT(setTimeLimit(Color,QTime)) );
                playerClock->setTimeLimit ( White, m_protocol->timeLimit ( White ) );
                playerClock->setTimeLimit ( Black, m_protocol->timeLimit ( Black ) );
            }
            else
            {
                playerClock->setTimeLimit ( playerColor, m_playerTime );
                playerClock->setTimeLimit ( oppositeColor ( playerColor ), m_oppTime );
            }

            playerClock->setDisplayedPlayer ( playerColor );
        }
        else
        {
            playerClock->setPlayerName ( White, i18n ( "White" ) );
            playerClock->setPlayerName ( Black, i18n ( "Black" ) );

            playerClock->setTimeLimit ( White, m_playerTime );
            playerClock->setTimeLimit ( Black, m_oppTime );

            playerClock->setDisplayedPlayer ( White );
        }
        addDockWidget ( Qt::RightDockWidgetArea, m_clockDock );
    }

    void MainWindow::protocolError ( Protocol::ErrorCode errorCode, const QString& errorString )
    {
        if ( errorCode != Protocol::UserCancelled )
        {
            KMessageBox::error ( this, errorString, Protocol::stringFromErrorCode ( errorCode ) );
        }
        delete m_protocol;
        fileNew();
    }

    void MainWindow::optionsPreferences()
    {
        if ( KConfigDialog::showDialog ( QLatin1String ( "settings" ) ) )
        {
            return;
        }
        KConfigDialog *dialog = new KConfigDialog ( this, QLatin1String ( "settings" ), Settings::self() );
        QWidget *generalSettingsDlg = new QWidget;
        ui_prefs_base.setupUi ( generalSettingsDlg );
#if not defined WITH_ANIMATIONS
        ui_prefs_base.animationGroup->hide();
#endif
        dialog->addPage ( generalSettingsDlg, i18n ( "General" ), QLatin1String ( "games-config-options" ) );
        connect ( dialog, SIGNAL ( settingsChanged ( QString ) ), m_view, SLOT ( settingsChanged() ) );
        QWidget* themeDlg = new KGameThemeSelector ( dialog, Settings::self(), KGameThemeSelector::NewStuffEnableDownload );
        dialog->addPage ( themeDlg, i18n ( "Theme" ), QLatin1String ( "games-config-theme" ) );
        dialog->setAttribute ( Qt::WA_DeleteOnClose );
        dialog->show();
    }

    void MainWindow::pauseGame ( bool pause )
    {
        kDebug();
        m_view->setPaused ( pause );
        if ( m_protocol->supportedFeatures() & Protocol::Pause )
        {
            pause ? m_protocol->pauseGame() : m_protocol->resumeGame();
        }
    }
}

#include "knights.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
