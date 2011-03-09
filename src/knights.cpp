/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>

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
#include "gamemanager.h"
#include "rules/chessrules.h"

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
        connect ( m_view, SIGNAL ( gameNew() ), this, SLOT ( fileNew() ), Qt::QueuedConnection );
        connect ( Manager::self(), SIGNAL(initComplete()), SLOT(protocolInitSuccesful()) );

        QTimer::singleShot ( 0, this, SLOT ( fileNew() ) );
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::setupActions()
    {
        KStandardGameAction::gameNew ( this, SLOT ( fileNew() ), actionCollection() );
        KStandardGameAction::quit ( qApp, SLOT ( closeAllWindows() ), actionCollection() );
        KStandardGameAction::pause ( this, SLOT ( pauseGame ( bool ) ), actionCollection() );
        KStandardAction::preferences ( this, SLOT ( optionsPreferences() ), actionCollection() );

        KAction* resignAction = actionCollection()->addAction ( QLatin1String("resign"), Manager::self(), SLOT ( resign() ) );
        resignAction->setText ( i18n ( "Resign" ) );
        resignAction->setHelpText(i18n("Admit your inevitable defeat"));
        resignAction->setIcon(KIcon(QLatin1String("flag-red")));

        KAction* undoAction = actionCollection()->addAction ( QLatin1String("move_undo"), this, SLOT(undo()) );
        undoAction->setText ( i18n("Undo") );
        undoAction->setHelpText ( i18n("Take back your last move") );
        undoAction->setIcon ( KIcon(QLatin1String("edit-undo")) );;
        undoAction->setEnabled(false);
        connect ( Manager::self(), SIGNAL(undoPossible(bool)), undoAction, SLOT(setEnabled(bool)) );

        KAction* redoAction = actionCollection()->addAction ( QLatin1String("move_redo"), this, SLOT(redo()) );
        redoAction->setText ( i18n("Redo") );
        redoAction->setHelpText ( i18n("Repeat your last move") );
        redoAction->setIcon ( KIcon(QLatin1String("edit-redo")) );;
        redoAction->setEnabled(false);
        connect ( Manager::self(), SIGNAL(redoPossible(bool)), redoAction, SLOT(setEnabled(bool)) );

        connect( qApp, SIGNAL( lastWindowClosed() ), this, SLOT( exitKnights() ) );
    }

    void MainWindow::fileNew()
    {
        KDialog gameNewDialog;
        GameDialog* dialogWidget = new GameDialog ( &gameNewDialog );
        gameNewDialog.setMainWidget ( dialogWidget );
        gameNewDialog.setCaption ( i18n ( "New Game" ) );
        if ( gameNewDialog.exec() == KDialog::Accepted )
        {
            Manager::self()->reset();
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
            dialogWidget->setupProtocols();
            connect ( Protocol::white(), SIGNAL(error(Protocol::ErrorCode,QString)), SLOT(protocolError(Protocol::ErrorCode,QString)) );
            connect ( Protocol::black(), SIGNAL(error(Protocol::ErrorCode,QString)), SLOT(protocolError(Protocol::ErrorCode,QString)) );
            dialogWidget->writeConfig();
            
            Manager::self()->initialize();
        }
    }

void MainWindow::showFicsDialog(Color color, bool computer)
{
    if ( computer || true) // TODO: Implement, and remove this check
    {
        KMessageBox::sorry(this, i18n("This feature is not yet implemented in Knights"));
        QTimer::singleShot(1, this, SLOT(fileNew()));
        return;
    }
    FicsProtocol* p = new FicsProtocol();
    p->setColor(color);
    p->init();
    p->openGameDialog();
}

void MainWindow::showFicsSpectateDialog()
{
    KMessageBox::sorry(this, i18n("This feature is not yet implemented in Knights"));
    QTimer::singleShot(1, this, SLOT(fileNew()));
    return;
}

    void MainWindow::protocolInitSuccesful()
    {
        QString whiteName = Protocol::white()->playerName();
        QString blackName = Protocol::black()->playerName();
        setCaption( i18n ( "%1 vs. %2", whiteName, blackName ) );
        if ( Manager::self()->timeControlEnabled ( White ) || Manager::self()->timeControlEnabled ( Black ) )
        {
            showClockWidgets();
            
            KToggleAction* clockAction = new KToggleAction ( KIcon(QLatin1String("clock")), i18n("Show Clock"), actionCollection() );
            actionCollection()->addAction ( QLatin1String("show_clock"), clockAction );
            connect ( clockAction, SIGNAL(triggered(bool)), m_clockDock, SLOT(setVisible(bool)) );
            connect ( clockAction, SIGNAL(triggered(bool)), this, SLOT(setShowClockSetting(bool)) );
            connect ( m_clockDock, SIGNAL(visibilityChanged(bool)), clockAction, SLOT(setChecked(bool)) );
            m_protocolActions << clockAction;
        }

        Protocol* player = 0;
        Protocol* opp = 0;
        if ( Protocol::white()->isLocal() )
        {
            player = Protocol::white();
            opp = Protocol::black();
        }
        if ( Protocol::black()->isLocal() )
        {
            if ( player || opp )
            {
                player = 0;
                opp = 0;
            }
            else
            {
                player = Protocol::black();
                opp = Protocol::white();
            }
        }
        if ( !player )
        {
            // Either two local humans, or two computers / FICS players
            // in both cases, we don't need those actions
        }
        else
        {
            Protocol::Features f = opp->supportedFeatures();
            if ( f & Protocol::Draw )
            {
                KAction* drawAction = actionCollection()->addAction ( QLatin1String ( "propose_draw" ), Manager::self(), SLOT ( offerDraw() ) );
                drawAction->setText ( i18n ( "Offer &Draw" ) );
                drawAction->setHelpText(i18n("Offer a draw to your opponent"));
                drawAction->setIcon(KIcon(QLatin1String("flag-blue")));
                m_protocolActions << drawAction;
            }
            if ( f & Protocol::Adjourn )
            {
                KAction* adjournAction = actionCollection()->addAction ( QLatin1String ( "adjourn" ), Manager::self(), SLOT ( adjourn() ) );
                adjournAction->setText ( i18n ( "Adjourn" ) );
                adjournAction->setHelpText(i18n("Continue this game at a later time"));
                adjournAction->setIcon(KIcon(QLatin1String("document-save")));
                m_protocolActions << adjournAction;
            }
        }
        QList<Protocol::ToolWidgetData> list;
        list << Protocol::black()->toolWidgets();
        list << Protocol::white()->toolWidgets();
        foreach ( const Protocol::ToolWidgetData& data, list )
            {
                QDockWidget* dock = new QDockWidget ( data.title, this );
                dock->setWidget ( data.widget );
                dock->setObjectName ( data.name + QLatin1String("DockWidget") );
                QString iconName;
                QString actionName;
                QString actionText;
                bool show = false;
               
                switch ( data.type )
                {
                    case Protocol::ConsoleToolWidget:
                        iconName = QLatin1String("utilities-terminal");
                        actionName = QLatin1String( data.owner == White ? "show_console_white" : "show_console_black" );
                        actionText = i18n("Show Console");
                        show = Settings::showConsole();
                        break;
                        
                    case Protocol::ChatToolWidget:
                        iconName = QLatin1String("meeting-attending");
                        actionName = QLatin1String("show_chat");
                        actionText = i18n("Show Chat");
                        show = Settings::showChat();
                        break;
                        
                    default:
                        actionName = data.name;
                        actionText = data.title;
                        break;
                }
        
                KToggleAction* toolAction = new KToggleAction ( KIcon(iconName), actionText, actionCollection() );
                connect ( toolAction, SIGNAL(triggered(bool)), dock, SLOT(setVisible(bool)) );
                switch ( data.type )
                {
                    case Protocol::ConsoleToolWidget:
                        connect ( toolAction, SIGNAL(triggered(bool)), this, SLOT(setShowConsoleSetting(bool)) );
                        break;
                    case Protocol::ChatToolWidget:
                        connect ( toolAction, SIGNAL(triggered(bool)), this, SLOT(setShowChatSetting(bool)) );
                        break;
                        
                    default:
                        break;
                }
                connect ( dock, SIGNAL(visibilityChanged(bool)), toolAction, SLOT(setChecked(bool)) );
                actionCollection()->addAction ( actionName, toolAction );
                m_protocolActions << toolAction;
                
                m_dockWidgets << dock;
                addDockWidget ( Qt::LeftDockWidgetArea, dock );
                dock->setVisible ( show );
            }
        QAction* wc = actionCollection()->action ( QLatin1String("show_console_white") );
        QAction* bc = actionCollection()->action ( QLatin1String("show_console_black") );
        if ( wc && bc )
        {
            wc->setText ( i18n("Show White Console") );
            bc->setText ( i18n("Show Black Console") );
        }
        createGUI(); // Resets dock widget states
        
        QAction* a = actionCollection()->action ( QLatin1String("show_clock") );
        if ( a )
        {
            a->setChecked ( Settings::showClock() );
        }
        a = actionCollection()->action ( QLatin1String("show_chat") );
        if ( a )
        {
            a->setChecked ( Settings::showChat() );
        }
        
        if ( wc )
        {
            wc->setChecked ( Settings::showConsole() );
        }
        if ( bc )
        {
            bc->setChecked ( Settings::showConsole() );
        }
        
        Manager::self()->setRules ( new ChessRules );
        Manager::self()->startGame();
        m_view->setupBoard();
    }
    
    void MainWindow::showClockWidgets()
    {
        ClockWidget* playerClock = new ClockWidget ( this );
        m_clockDock = new QDockWidget ( i18n ( "Clock" ), this );
        m_clockDock->setObjectName ( QLatin1String ( "ClockDockWidget" ) ); // for QMainWindow::saveState()
        m_clockDock->setWidget ( playerClock );
        m_dockWidgets << m_clockDock;
        
        connect ( m_view, SIGNAL ( displayedPlayerChanged ( Color ) ), playerClock, SLOT ( setDisplayedPlayer ( Color ) ) );
        
        playerClock->setPlayerName(White, Protocol::white()->playerName());
        playerClock->setPlayerName(Black, Protocol::black()->playerName());

        connect ( Manager::self(), SIGNAL(timeChanged(Color,QTime)), playerClock, SLOT(setCurrentTime(Color,QTime)) );

        playerClock->setTimeLimit ( White, Manager::self()->timeLimit ( White ) );
        playerClock->setTimeLimit ( Black, Manager::self()->timeLimit ( Black ) );

        addDockWidget ( Qt::RightDockWidgetArea, m_clockDock );
    }

    void MainWindow::protocolError ( Protocol::ErrorCode errorCode, const QString& errorString )
    {
        if ( errorCode != Protocol::UserCancelled )
        {
            KMessageBox::error ( this, errorString, Protocol::stringFromErrorCode ( errorCode ) );
        }
        Protocol::white()->deleteLater();
        Protocol::black()->deleteLater();
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
        m_view->setPaused ( pause );
        Offer o;
        o.action = pause ? ActionPause : ActionResume;
        o.id = qrand();
        o.player = NoColor;
        Manager::self()->sendOffer(o);
    }
    
    void MainWindow::undo()
    {
        if ( !Protocol::white()->isLocal() && !Protocol::black()->isLocal() )
        {
            // No need to pause the game if both players are local
            QAction* pa = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));
            if ( pa )
            {
                pa->setChecked(true);
            }
        }
        Manager::self()->undo();
    }
    
    void MainWindow::redo()
    {
        Manager::self()->redo();
        if ( !Protocol::white()->isLocal() && !Protocol::black()->isLocal() )
        {
            // No need to pause the game if both players are local
            QAction* pa = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));
            if ( pa && !Manager::self()->canRedo() )
            {
                pa->setChecked(false);
            }
        }
    }

    void MainWindow::setShowClockSetting( bool value )
    {
        Settings::self()->setShowClock( value );
    }

    void MainWindow::setShowConsoleSetting( bool value )
    {
        Settings::self()->setShowConsole( value );
    }

    void MainWindow::setShowChatSetting( bool value )
    {
        Settings::self()->setShowChat( value );
    }

    void MainWindow::exitKnights()
    {
        //This will close the gnuchess/crafty/whatever process if it's running.
        if ( Protocol::white() ) {
            delete Protocol::white();
        }
        if ( Protocol::black() ) {
            delete Protocol::black();
        }
        Settings::self()->writeConfig();
    }

}

#include "knights.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
