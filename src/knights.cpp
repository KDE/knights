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
#include "gamemanager.h"

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

            switch ( dialogWidget->ficsMode() )
            {
                case GameDialog::NoFics:
                    dialogWidget->setupProtocols();
                    break;

                case GameDialog::PlayerVsFics:
                    showFicsDialog(dialogWidget->ficsColor(), false);
                    break;

                case GameDialog::ComputerVsFics:
                    showFicsDialog(dialogWidget->ficsColor(), true);
                    break;

                case GameDialog::BothPlayersFics:
                    showFicsSpectateDialog();
                    break;
            }
            Manager::self()->initialize();
            protocolInitSuccesful();
        }
    }

    void MainWindow::undo()
    {
        QAction* pauseAction = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));
        if ( pauseAction && !pauseAction->isChecked() )
        {
            pauseAction->trigger();
        }
        Manager::self()->undo();
    }

    void MainWindow::redo()
    {
        QAction* pauseAction = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));

        if ( pauseAction && !pauseAction->isChecked() )
        {
            pauseAction->trigger();
        }
        Manager::self()->redo();
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
            if ( f & Protocol::SetTimeLimit )
            {
                
                // The time limit was set or changed by the protocol
                m_timeLimit = opp->attribute ( QLatin1String ( "TimeLimitEnabled" ) ).toBool();
                if ( m_timeLimit )
                {
                    m_playerTime = opp->attribute ( QLatin1String ( "playerTime" ) ).toTime();
                    m_oppTime = opp->attribute ( QLatin1String ( "opponentTime" ) ).toTime();
                }
            }
            
            if ( f & Protocol::Draw )
            {
                KAction* drawAction = actionCollection()->addAction ( QLatin1String ( "propose_draw" ), opp, SLOT ( proposeDraw() ) );
                drawAction->setText ( i18n ( "Propose &Draw" ) );
                drawAction->setHelpText(i18n("Propose a draw to your opponent"));
                drawAction->setIcon(KIcon(QLatin1String("flag-blue")));
                m_protocolActions << drawAction;
                connect ( opp, SIGNAL(drawOffered()), SLOT(drawOffered()) );
            }
            if ( f & Protocol::Resign )
            {
                KAction* resignAction = actionCollection()->addAction ( QLatin1String ( "resign" ), opp, SLOT ( resign() ) );
                resignAction->setText ( i18n ( "Resign" ) );
                resignAction->setHelpText(i18n("Admit your inevitable defeat"));
                resignAction->setIcon(KIcon(QLatin1String("flag-red")));
                m_protocolActions << resignAction;
            }
            if ( f & Protocol::Adjourn )
            {
                KAction* adjournAction = actionCollection()->addAction ( QLatin1String ( "adjourn" ), opp, SLOT ( adjourn() ) );
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
                connect ( opp, SIGNAL(undoPossible(bool)), undoAction, SLOT(setEnabled(bool)) );
                m_protocolActions << undoAction;
                
                KAction* redoAction = KStandardAction::redo( this, SLOT(redo()), actionCollection() );
                redoAction->setEnabled(false);
                connect ( opp, SIGNAL(redoPossible(bool)), redoAction, SLOT(setEnabled(bool)) );
                m_protocolActions << redoAction;
            }
        }
        QList<Protocol::ToolWidgetData> list;
        list << Protocol::white()->toolWidgets();
        list << Protocol::black()->toolWidgets();
        foreach ( const Protocol::ToolWidgetData& data, list )
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
        
        playerClock->setPlayerName(White, Protocol::white()->playerName());
        playerClock->setPlayerName(Black, Protocol::black()->playerName());

        connect ( Manager::self(), SIGNAL(timeChanged(Color,QTime)), playerClock, SLOT(setCurrentTime(Color,QTime)) );
        connect ( Manager::self(), SIGNAL(timeLimitChanged(Color,QTime)), playerClock, SLOT(setTimeLimit(Color,QTime)) );

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
        pause ? Manager::self()->stopTime() : Manager::self()->startTime();
    }

    void MainWindow::drawOffered()
    {
        if ( KMessageBox::questionYesNo ( this,
            i18n( "Your opponent, %1, offers you a draw. Do you accept?", Protocol::white()->playerName() ),
            i18n( "Draw offer" )) == KMessageBox::Yes )
        {
            m_view->gameOver ( NoColor );
        }
    }

}

#include "knights.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
