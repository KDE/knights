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
#include <KGameThemeSelector>
#include <KStandardGameAction>
#include <KLocale>
#include <KMessageBox>

#include <QtGui/QDropEvent>
#include <QtCore/QTimer>
#include <QtGui/QDockWidget>

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
        KStandardGameAction::pause ( this, SLOT ( pauseGame ( bool ) ), actionCollection() );
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
            hideClockWidgets();
            delete m_protocol;
            dialogWidget->writeConfig();

            QVariantMap protocolOptions;
            switch ( dialogWidget->protocol() )
            {
                case Settings::EnumProtocol::XBoard:
                    m_protocol = new XBoardProtocol;
                    protocolOptions[QLatin1String ( "program" ) ] = dialogWidget->program();
                    break;
                case Settings::EnumProtocol::FICS:
                    m_protocol = new FicsProtocol;
                    protocolOptions[QLatin1String ( "server" ) ] = dialogWidget->server();
                    break;
                default:
                    break;
            }

            m_timeLimit = dialogWidget->timeLimit();
            if ( m_timeLimit )
            {
                m_playerTime = dialogWidget->playerTime();
                m_oppTime = dialogWidget->opponentTime();
                m_playerIncrement = dialogWidget->playerIncrement();
                m_oppIncrement = dialogWidget->opponentIncrement();
                if ( m_protocol )
                {
                    protocolOptions[QLatin1String ( "playerTimeLimit" ) ] = m_playerTime;
                    protocolOptions[QLatin1String ( "playerTimeIncrement" ) ] = m_playerIncrement;
                    protocolOptions[QLatin1String ( "opponentTimeLimit" ) ] = m_oppTime;
                    protocolOptions[QLatin1String ( "opponentTimeIncrement" ) ] = m_oppIncrement;
                }
            }

            if ( m_protocol )
            {
                protocolOptions[QLatin1String ( "PlayerColor" ) ] = QVariant::fromValue<Color> ( dialogWidget->color() );
                connect ( m_protocol, SIGNAL ( initSuccesful() ), SLOT ( protocolInitSuccesful() ), Qt::QueuedConnection );
                connect ( m_protocol, SIGNAL ( error ( Protocol::ErrorCode, QString ) ), SLOT ( protocolError ( Protocol::ErrorCode, QString ) ), Qt::QueuedConnection );
                m_protocol->init ( protocolOptions );
            }
            else
            {
                QTimer::singleShot ( 0, this, SLOT ( protocolInitSuccesful() ) );
            }
        }
    }

    void MainWindow::protocolInitSuccesful()
    {
        if ( m_protocol && m_protocol->supportedFeatures() & Protocol::SetTimeLimit )
        {
            // The time limit was set or changed by the protocol
            m_timeLimit = m_protocol->attribute ( QLatin1String ( "TimeLimitEnabled" ) ).toBool();
            if ( m_timeLimit )
            {
                m_playerTime = m_protocol->attribute ( QLatin1String ( "playerTime" ) ).toTime();
                m_oppTime = m_protocol->attribute ( QLatin1String ( "opponentTime" ) ).toTime();
            }
        }
        if ( m_timeLimit )
        {
            showClockWidgets();
        }
        m_view->setupBoard ( m_protocol );
    }

    void MainWindow::showClockWidgets()
    {
        ClockWidget* playerClock = new ClockWidget ( this );
        m_clockDock = new QDockWidget ( i18n ( "Clock" ), this );
        m_clockDock->setObjectName ( QLatin1String ( "ClockDockWidget" ) ); // for QMainWindow::saveState()
        m_clockDock->setWidget ( playerClock );

        connect ( m_view, SIGNAL ( activePlayerChanged ( Color ) ), playerClock, SLOT ( setActivePlayer ( Color ) ) );
        connect ( m_view, SIGNAL ( displayedPlayerChanged ( Color ) ), playerClock, SLOT ( setDisplayedPlayer ( Color ) ) );

        bool protocolEmitsGameOver = m_protocol && m_protocol->supportedFeatures() & Protocol::GameOver;
        if ( !protocolEmitsGameOver )
        {
            connect ( playerClock, SIGNAL ( opponentTimeOut ( Color ) ), m_view, SLOT ( gameOver ( Color ) ) );
        }
        if ( m_protocol )
        {
            Color playerColor = m_protocol->playerColor();
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
                connect ( m_protocol, SIGNAL ( timeChanged ( Color, QTime ) ), playerClock, SLOT ( setCurrentTime ( Color, QTime ) ) );
            }
            else
            {
                playerClock->setTimeLimit ( playerColor, m_playerTime );
                playerClock->setTimeLimit ( oppositeColor ( playerColor ), m_oppTime );

                playerClock->setTimeIncrement ( playerColor, m_oppIncrement );
                playerClock->setTimeIncrement ( oppositeColor ( playerColor ), m_oppIncrement );
            }
            playerClock->setDisplayedPlayer ( playerColor );
        }
        else
        {
            playerClock->setPlayerName ( White, i18n ( "White" ) );
            playerClock->setPlayerName ( Black, i18n ( "Black" ) );

            playerClock->setTimeLimit ( White, m_playerTime );
            playerClock->setTimeIncrement ( White, m_playerIncrement );
            playerClock->setTimeLimit ( Black, m_oppTime );
            playerClock->setTimeIncrement ( Black, m_oppIncrement );

            playerClock->setDisplayedPlayer ( White );
        }
        playerClock->setActivePlayer ( White );
        addDockWidget ( Qt::RightDockWidgetArea, m_clockDock );
    }

    void MainWindow::hideClockWidgets()
    {
        removeDockWidget ( m_clockDock );
        delete m_clockDock;
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
        if ( m_protocol && m_protocol->supportedFeatures() & Protocol::Pause )
        {
            pause ? m_protocol->pauseGame() : m_protocol->resumeGame();
        }
        if ( m_clockDock )
        {
            ClockWidget* clock = qobject_cast< ClockWidget* > ( m_clockDock->widget() );
            if ( clock )
            {
                pause ? clock->pauseClock() : clock->resumeClock();
            }
        }
    }
}

#include "knights.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
