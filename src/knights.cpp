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
    KStandardGameAction::pause( this, SLOT(pauseGame(bool)), actionCollection());
    KStandardGameAction::quit ( qApp, SLOT ( closeAllWindows() ), actionCollection() );

    KStandardAction::preferences ( this, SLOT ( optionsPreferences() ), actionCollection() );
}

void MainWindow::fileNew()
{
    KDialog* gameNewDialog = new KDialog;
    GameDialog* dialogWidget = new GameDialog ( gameNewDialog );
    gameNewDialog->setMainWidget ( dialogWidget );
    gameNewDialog->setCaption(i18n("New Game"));
    if ( gameNewDialog->exec() == KDialog::Accepted ) {
        removeDockWidget(m_clockDock);
        delete m_clockDock;
        delete m_protocol;

        Piece::Color playerColor = dialogWidget->color();
        QList<Piece::Color> playerColors;
        QVariantMap protocolOptions;
        switch ( dialogWidget->protocol() ) {
        case Settings::EnumProtocol::XBoard:
            m_protocol = new XBoardProtocol;
            protocolOptions["program"] = dialogWidget->program();
            break;
        default:
            playerColors << Piece::White << Piece::Black;
            break;
        }

        if ( m_protocol ) {
            if ( playerColor == Piece::NoColor ) {
                playerColor = ( qrand() % 2 == 0 ) ? Piece::White : Piece::Black;
                kDebug() << playerColor;
            }
            playerColors << playerColor;
            m_protocol->setPlayerColor ( playerColor );
            if ( !m_protocol->init (  protocolOptions ) ) {
                KMessageBox::error ( this, i18n ( "The selected program, <code>%1</code>, could not be started", dialogWidget->program() ) );
                return;
            }
        } else {
            playerColors << Piece::White << Piece::Black;
        }
        m_view->setupBoard ( m_protocol, playerColors );

        if ( dialogWidget->timeLimit() ) {
            ClockWidget* playerClock = new ClockWidget ( this );
            m_clockDock = new QDockWidget ( i18n("Clock"), this );
            m_clockDock->setObjectName("ClockDockWidget"); // for QMainWindow::saveState()
            m_clockDock->setWidget ( playerClock );

            connect ( m_view, SIGNAL ( activePlayerChanged ( Piece::Color ) ), playerClock, SLOT ( setActivePlayer ( Piece::Color ) ) );
            connect ( m_view, SIGNAL ( displayedPlayerChanged ( Piece::Color ) ), playerClock, SLOT ( setDisplayedPlayer ( Piece::Color ) ) );

            connect ( playerClock, SIGNAL(opponentTimeOut(Piece::Color)), m_view, SLOT ( gameOver(Piece::Color)) );

            if ( m_protocol ) {
                playerClock->setPlayerName ( playerColor, i18n ( "You" ) );
                playerClock->setPlayerName ( Piece::oppositeColor ( playerColor ), i18n ( "Opponent" ) );
                playerClock->setTimeLimit ( playerColor, dialogWidget->playerTime() );
                playerClock->setTimeLimit ( Piece::oppositeColor ( playerColor ), dialogWidget->opponentTime() );

                playerClock->setDisplayedPlayer(playerColor);
                // TODO: Get names from dialog and protocol
            } else {
                playerClock->setPlayerName ( Piece::White, i18n ( "White" ) );
                playerClock->setPlayerName ( Piece::Black, i18n ( "Black" ) );

                playerClock->setTimeLimit ( Piece::White, dialogWidget->playerTime() );
                playerClock->setTimeLimit ( Piece::Black, dialogWidget->opponentTime() );

                playerClock->setDisplayedPlayer(Piece::White);

            }
            playerClock->setActivePlayer ( Piece::White );
            addDockWidget ( Qt::RightDockWidgetArea, m_clockDock );

        }
    }
    delete gameNewDialog;
}

void MainWindow::optionsPreferences()
{
    // The preference dialog is derived from prefs_base.ui
    //
    // compare the names of the widgets in the .ui file
    // to the names of the variables in the .kcfg file
    //avoid to have 2 dialogs shown
    if ( KConfigDialog::showDialog ( "settings" ) )  {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog ( this, "settings", Settings::self() );
    QWidget *generalSettingsDlg = new QWidget;
    ui_prefs_base.setupUi ( generalSettingsDlg );
    dialog->addPage ( generalSettingsDlg, i18n ( "General" ), "config" );
    connect ( dialog, SIGNAL ( settingsChanged ( QString ) ), m_view, SLOT ( settingsChanged() ) );
    QWidget* themeDlg = new KGameThemeSelector ( dialog, Settings::self(), KGameThemeSelector::NewStuffDisableDownload );
    dialog->addPage ( themeDlg, i18n ( "Theme" ), "games-config-theme" );
    dialog->setAttribute ( Qt::WA_DeleteOnClose );
    dialog->show();
}

void MainWindow::pauseGame(bool pause)
{
    kDebug();
    m_view->setPaused(pause);
    if (m_protocol && m_protocol->supportedFeatures() & Protocol::Pause)
    {
        pause ? m_protocol->pauseGame() : m_protocol->resumeGame();
    }
    if (m_clockDock)
    {
        ClockWidget* clock = qobject_cast< ClockWidget* >(m_clockDock->widget());
        if (clock)
        {
            pause ? clock->pauseClock() : clock->resumeClock();
        }
    }
}


}

#include "knights.moc"
