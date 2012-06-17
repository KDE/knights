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
#include "proto/xboardprotocol.h"
#include "proto/ficsprotocol.h"
#include "proto/localprotocol.h"
#include "rules/chessrules.h"
#include "gamemanager.h"
#include "knightsview.h"
#include "settings.h"
#include "gamedialog.h"
#include "clockwidget.h"
#include "historywidget.h"
#include "enginesettings.h"

#include <KConfigDialog>
#include <KStatusBar>
#include <KAction>
#include <KActionCollection>
#include <KStandardAction>
#include <KToggleAction>
#include <KLocale>
#include <KMessageBox>
#include <KFileDialog>
#include <KUser>
#include <KgDifficulty>
#include <KgThemeSelector>
#include <KStandardGameAction>

#include <QDropEvent>
#include <QTimer>
#include <QDockWidget>
#include <QListView>
#include <QStringListModel>

const char* DontAskDiscard = "dontAskInternal";

namespace Knights
{
    MainWindow::MainWindow()
            : KXmlGuiWindow(),
            m_view ( new KnightsView ( this ) ),
            m_clockDock ( 0 ),
            m_themeProvider ( new KgThemeProvider ( "Theme", this ) )
    {
        // accept dnd
        setAcceptDrops ( true );

        // tell the KXmlGuiWindow that this is indeed the main widget
        setCentralWidget ( m_view );
        
        // initial creation/setup of the docks
        setDockNestingEnabled (true);
        setupClockDock();
        setupConsoleDocks();
        setupHistoryDock();

        // then, setup our actions
        setupActions();

        // add a status bar
        statusBar()->show();
        
        connect (Kg::difficulty(), SIGNAL(currentLevelChanged(const KgDifficultyLevel*)), Manager::self(), SLOT(levelChanged(const KgDifficultyLevel*)));
        
        Kg::difficulty()->addLevel ( new KgDifficultyLevel ( 0, "custom", i18n("Custom"), false ) );
        Kg::difficulty()->addStandardLevelRange ( KgDifficultyLevel::VeryEasy, KgDifficultyLevel::VeryHard, KgDifficultyLevel::Medium );

        // a call to KXmlGuiWindow::setupGUI() populates the GUI
        // with actions, using KXMLGUI.
        // It also applies the saved mainwindow settings, if any, and ask the
        // mainwindow to automatically save settings if changed: window size,
        // toolbar position, icon size, etc.
        setupGUI();
        KgDifficultyGUI::init( this );
        
        // make all the docks invisible.
        // Show required docks after the game protocols are selected
        m_clockDock->hide();
        m_bconsoleDock->hide();
        m_wconsoleDock->hide();
        m_chatDock->hide();
        
        connect ( m_view, SIGNAL (gameNew()), this, SLOT (fileNew()), Qt::QueuedConnection );
        connect ( Manager::self(), SIGNAL (initComplete()), SLOT (protocolInitSuccesful()) );
        connect ( Manager::self(), SIGNAL (playerNameChanged()), SLOT (updateCaption()) );
        connect( qApp, SIGNAL (lastWindowClosed()), this, SLOT (exitKnights()) );
                
        QTimer::singleShot ( 0, this, SLOT (fileNew()) );
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::setupActions()
    {
        KStandardGameAction::gameNew ( this, SLOT (fileNew()), actionCollection() );
        KStandardGameAction::quit ( qApp, SLOT (closeAllWindows()), actionCollection() );
        pause = KStandardGameAction::pause ( this, SLOT (pauseGame(bool)), actionCollection() );
        pause->setEnabled ( false );
        KStandardAction::preferences ( this, SLOT (optionsPreferences()), actionCollection() );
        
        KStandardGameAction::save ( this, SLOT (fileSave()), actionCollection() );
        KStandardGameAction::saveAs ( this, SLOT (fileSaveAs()), actionCollection() );
        KStandardGameAction::load ( this, SLOT (fileLoad()), actionCollection() );
        
        KAction* resignAction = actionCollection()->addAction ( QLatin1String("resign"), Manager::self(), SLOT (resign()) );
        resignAction->setText ( i18n ( "Resign" ) );
        resignAction->setHelpText(i18n("Admit your inevitable defeat"));
        resignAction->setIcon(KIcon(QLatin1String("flag-red")));
        resignAction->setEnabled( false );

        KAction* undoAction = actionCollection()->addAction ( QLatin1String("move_undo"), this, SLOT(undo()) );
        undoAction->setText ( i18n("Undo") );
        undoAction->setHelpText ( i18n("Take back your last move") );
        undoAction->setIcon ( KIcon(QLatin1String("edit-undo")) );;
        connect ( Manager::self(), SIGNAL (undoPossible(bool)), undoAction, SLOT (setEnabled(bool)) );
        undoAction->setEnabled( false );

        KAction* redoAction = actionCollection()->addAction ( QLatin1String("move_redo"), this, SLOT(redo()) );
        redoAction->setText ( i18n("Redo") );
        redoAction->setHelpText ( i18n("Repeat your last move") );
        redoAction->setIcon ( KIcon(QLatin1String("edit-redo")) );;
        connect ( Manager::self(), SIGNAL (redoPossible(bool)), redoAction, SLOT (setEnabled(bool)) );
        redoAction->setEnabled( false );
        
        KAction* drawAction = actionCollection()->addAction ( QLatin1String ( "propose_draw" ), Manager::self(), SLOT (offerDraw()) );
        drawAction->setText ( i18n ( "Offer &Draw" ) );
        drawAction->setHelpText(i18n("Offer a draw to your opponent"));
        drawAction->setIcon(KIcon(QLatin1String("flag-blue")));
        drawAction->setEnabled( false );
        
        KAction* adjournAction = actionCollection()->addAction ( QLatin1String ( "adjourn" ), Manager::self(), SLOT (adjourn()) );
        adjournAction->setText ( i18n ( "Adjourn" ) );
        adjournAction->setHelpText(i18n("Continue this game at a later time"));
        adjournAction->setIcon(KIcon(QLatin1String("document-save")));
        adjournAction->setEnabled( false );
        
        KAction* abortAction = actionCollection()->addAction ( QLatin1String("abort"), Manager::self(), SLOT (abort()) );
        abortAction->setText ( i18n ( "Abort" ) );
        abortAction->setHelpText(i18n("End the game immediately"));
        abortAction->setIcon(KIcon(QLatin1String("dialog-cancel")));
        abortAction->setEnabled( false );
        
        KToggleAction* clockAction = new KToggleAction ( KIcon(QLatin1String("clock")), i18n("Show Clock"), actionCollection() );
        actionCollection()->addAction ( QLatin1String("show_clock"), clockAction );
        connect ( clockAction, SIGNAL (triggered(bool)), m_clockDock, SLOT (setVisible(bool)) );
        connect ( clockAction, SIGNAL (triggered(bool)), this, SLOT (setShowClockSetting(bool)) );
        clockAction->setVisible (false);
        
        KToggleAction* historyAction = new KToggleAction ( KIcon(QLatin1String("view-history")), i18n("Show History"), actionCollection() );
        actionCollection()->addAction ( QLatin1String("show_history"), historyAction );
        connect ( historyAction, SIGNAL (triggered(bool)), m_historyDock, SLOT (setVisible(bool)) );
        connect ( historyAction, SIGNAL (triggered(bool)), this, SLOT (setShowHistorySetting(bool)) );
        historyAction->setVisible (true);
        historyAction->setChecked ( Settings::showHistory() );
      
        KToggleAction* wconsoleAction = new KToggleAction ( KIcon(QLatin1String("utilities-terminal")), i18n("Show White Console"), actionCollection() );
        actionCollection()->addAction ( QLatin1String("show_console_white"), wconsoleAction );
        connect ( wconsoleAction, SIGNAL (triggered(bool)), m_wconsoleDock, SLOT (setVisible(bool)) );
        connect ( wconsoleAction, SIGNAL (triggered(bool)), this, SLOT (setShowConsoleSetting()) );
        wconsoleAction->setVisible (false);
        
        KToggleAction* bconsoleAction = new KToggleAction ( KIcon(QLatin1String("utilities-terminal")), i18n("Show Black Console"), actionCollection() );
        actionCollection()->addAction ( QLatin1String("show_console_black"), bconsoleAction );
        connect ( bconsoleAction, SIGNAL (triggered(bool)), m_bconsoleDock, SLOT (setVisible(bool)) );
        connect ( bconsoleAction, SIGNAL (triggered(bool)), this, SLOT (setShowConsoleSetting()) );
        bconsoleAction->setVisible (false);        
        
        KToggleAction* chatAction = new KToggleAction ( KIcon(QLatin1String("meeting-attending")), i18n("Show Chat"), actionCollection() );
        actionCollection()->addAction ( QLatin1String("show_chat"), chatAction );
        connect ( chatAction, SIGNAL (triggered(bool)), m_chatDock, SLOT (setVisible(bool)) );
        connect ( chatAction, SIGNAL (triggered(bool)), this, SLOT (setShowChatSetting(bool)) );
        chatAction->setVisible (false); 
        
        protocolFeatures [ KStandardGameAction::name(KStandardGameAction::Pause) ] = Protocol::Pause;
        protocolFeatures [ "propose_draw" ] = Protocol::Draw;
        protocolFeatures [ "adjourn" ] = Protocol::Adjourn;
        protocolFeatures [ "resign" ] = Protocol::Resign;
        protocolFeatures [ "abort" ] = Protocol::Abort;
    }

    void MainWindow::fileNew()
    {
        if (!maybeSave())
        {
            return;
        }
        KDialog gameNewDialog;
        GameDialog* dialogWidget = new GameDialog ( &gameNewDialog );
        gameNewDialog.setMainWidget ( dialogWidget );
        gameNewDialog.setCaption ( i18n ( "New Game" ) );
        if ( gameNewDialog.exec() == KDialog::Accepted )
        {
            Manager::self()->reset();
            m_view->clearBoard();
            dialogWidget->setupProtocols();
            connect ( Protocol::white(), SIGNAL(error(Protocol::ErrorCode,QString)), SLOT(protocolError(Protocol::ErrorCode,QString)) );
            connect ( Protocol::black(), SIGNAL(error(Protocol::ErrorCode,QString)), SLOT(protocolError(Protocol::ErrorCode,QString)) );
            dialogWidget->writeConfig();
            
            Manager::self()->initialize();
        }
    }
    
void MainWindow::fileLoad()
{
    if (!maybeSave())
    {
        return;
    }
    
    QString fileName = KFileDialog::getOpenFileName ( KUrl("kfiledialog://knights"), i18n("*.pgn | Portable game notation" ) );
    if ( fileName.isEmpty() )
    {
        return;
    }
    
    Manager::self()->reset();
    m_view->clearBoard();
    
    Protocol::setWhiteProtocol ( new LocalProtocol() );
    Protocol::setBlackProtocol ( new LocalProtocol() );
    
    connect ( Protocol::white(), SIGNAL(error(Protocol::ErrorCode,QString)), SLOT(protocolError(Protocol::ErrorCode,QString)) );
    connect ( Protocol::black(), SIGNAL(error(Protocol::ErrorCode,QString)), SLOT(protocolError(Protocol::ErrorCode,QString)) );
    
    m_loadFileName = fileName;
    Manager::self()->initialize();
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
        kDebug() << Settings::showClock() << Settings::showConsole();
        QString whiteName = Protocol::white()->playerName();
        QString blackName = Protocol::black()->playerName();
        updateCaption();
        
        // show clock action button if timer active
        // show clock dock widget if timer active and configuration file entry has visible = true
        if ( Manager::self()->timeControlEnabled ( White ) || Manager::self()->timeControlEnabled ( Black ) )
        {
            actionCollection()->action ( QLatin1String ( "show_clock" ) )->setVisible (true);
            playerClock->setPlayerName(White, Protocol::white()->playerName());
            playerClock->setPlayerName(Black, Protocol::black()->playerName());
            playerClock->setTimeLimit ( White, Manager::self()->timeLimit ( White ) );
            playerClock->setTimeLimit ( Black, Manager::self()->timeLimit ( Black ) );
            if (Settings::showClock())
            {
                m_clockDock->setVisible (true);
                actionCollection()->action ( QLatin1String ( "show_clock" ) )->setChecked (true);
            }
            else
            {
                m_clockDock->setVisible (false);
                actionCollection()->action ( QLatin1String ( "show_clock" ) )->setChecked (false);
            }
        }
        else
        {
            m_clockDock->setVisible (false);
            actionCollection()->action ( QLatin1String ( "show_clock" ) )->setVisible (false);
        }

        Protocol::Features f = Protocol::NoFeatures;
        if ( Protocol::white()->isLocal() && !(Protocol::black()->isLocal()) )
        {
            f = Protocol::black()->supportedFeatures();
        }
        else if ( Protocol::black()->isLocal() && !(Protocol::white()->isLocal()) )
        {
            f = Protocol::white()->supportedFeatures();
        }
        else if ( !(Protocol::black()->isLocal()) && !(Protocol::white()->isLocal()) )
        {
            // These protocol features make sense when neither player is local
            f = Protocol::Pause | Protocol::Adjourn | Protocol::Abort;
            f &= Protocol::white()->supportedFeatures();
            f &= Protocol::black()->supportedFeatures();
        }
                
        QMap<QByteArray, Protocol::Feature>::ConstIterator it;
        for ( it = protocolFeatures.constBegin(); it != protocolFeatures.constEnd(); ++it )
        {
            actionCollection()->action( QLatin1String(it.key()) )->setEnabled ( f & it.value() );
        }
        
        // show console action button if protocol allows a console
        // show console dock widget if protocol allows and configuration file entry has visible = true
        // finally, hide any dock widget not needed - in case it is still active from previous game
        actionCollection()->action ( QLatin1String ( "show_console_white" ) )->setVisible (false);
        actionCollection()->action ( QLatin1String ( "show_console_black" ) )->setVisible (false);
        actionCollection()->action ( QLatin1String ( "show_chat" ) )->setVisible (false);
        QList<Protocol::ToolWidgetData> list;
        list << Protocol::black()->toolWidgets();
        list << Protocol::white()->toolWidgets();
        foreach ( const Protocol::ToolWidgetData& data, list )
            {
                switch ( data.type )
                {
                    case Protocol::ConsoleToolWidget:
                        if( data.owner == White )
                        {
                            m_wconsoleDock->setWindowTitle ( data.title );
                            m_wconsoleDock->setWidget ( data.widget );
                            actionCollection()->action ( QLatin1String ( "show_console_white" ) )->setVisible (true);
                            if( Settings::showConsole() ) 
                            {
                                m_wconsoleDock->setVisible (true);
                                actionCollection()->action ( QLatin1String ( "show_console_white" ) )->setChecked (true);
                            }
                            else
                            {
                                m_wconsoleDock->setVisible (false);
                                actionCollection()->action ( QLatin1String ( "show_console_white" ) )->setChecked (false);
                            }
                        }
                        else
                        {
                            m_bconsoleDock->setWindowTitle ( data.title );
                            m_bconsoleDock->setWidget ( data.widget );
                            actionCollection()->action ( QLatin1String ( "show_console_black" ) )->setVisible (true);
                            if( Settings::showConsole() ) 
                            {
                                m_bconsoleDock->setVisible (true);
                                actionCollection()->action ( QLatin1String ( "show_console_black" ) )->setChecked (true);
                            }
                            else
                            {
                                m_bconsoleDock->setVisible (false);
                                actionCollection()->action ( QLatin1String ( "show_console_black" ) )->setChecked (false);
                            }
                        }
                        break;
                        
                    case Protocol::ChatToolWidget:
                        m_chatDock->setWindowTitle ( data.title );
                        m_chatDock->setWidget ( data.widget );
                        actionCollection()->action ( QLatin1String ( "show_chat" ) )->setVisible (true);
                        if( Settings::showChat() ) 
                        {
                            m_chatDock->setVisible (true);
                            actionCollection()->action ( QLatin1String ( "show_chat" ) )->setChecked (true);
                        }
                        else
                        {
                            m_chatDock->setVisible (false);
                            actionCollection()->action ( QLatin1String ( "show_chat" ) )->setChecked (false);
                        }
                        break;
                        
                    default:
                        break;
                }
            }
        if (!actionCollection()->action( QLatin1String ( "show_console_white" ) )->isVisible() ) 
        {
            m_wconsoleDock->hide();
        }
        if (!actionCollection()->action( QLatin1String ( "show_console_black" ) )->isVisible() )  
        {
            m_bconsoleDock->hide();
        }
        if (!actionCollection()->action( QLatin1String ( "show_chat" ) )->isVisible() )
        {
            m_chatDock->hide();
        }
        
        Manager::self()->setRules ( new ChessRules );
        Manager::self()->startGame();
        
        m_themeProvider->discoverThemes ( "appdata", QLatin1String ( "themes" ) );
        
        if (m_loadFileName.isEmpty())
        {
            m_view->setupBoard(m_themeProvider);
        } 
        else 
        {
            int speed = Settings::animationSpeed();
            Settings::setAnimationSpeed ( Settings::EnumAnimationSpeed::Instant );
            m_view->setupBoard(m_themeProvider);
            
            Manager::self()->loadGameHistoryFrom ( m_loadFileName );
            setCaption ( m_loadFileName );
            
            m_loadFileName.clear();
            Settings::setAnimationSpeed ( speed );
        }
    }
    
    void MainWindow::setupClockDock()
    {
        m_clockDock = new QDockWidget ( i18n ( "Clock" ), this );
        m_clockDock->setObjectName ( QLatin1String ( "ClockDockWidget" ) ); // for QMainWindow::saveState()
        playerClock = new ClockWidget ( this );  
        m_clockDock->setWidget ( playerClock );
        m_clockDock->setFeatures ( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
        connect ( m_view, SIGNAL (displayedPlayerChanged(Color)), playerClock, SLOT (setDisplayedPlayer(Color)) );
        connect ( Manager::self(), SIGNAL (timeChanged(Color,QTime)), playerClock, SLOT (setCurrentTime(Color,QTime)) );
        addDockWidget ( Qt::RightDockWidgetArea, m_clockDock );
    }
    
    void MainWindow::setupConsoleDocks()
    {
        m_bconsoleDock = new QDockWidget ();
        m_bconsoleDock->setObjectName ( QLatin1String ( "BlackConsoleDockWidget" ) );
        m_bconsoleDock->setFeatures ( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
        addDockWidget ( Qt::LeftDockWidgetArea, m_bconsoleDock );
          
        m_wconsoleDock = new QDockWidget ();
        m_wconsoleDock->setObjectName ( QLatin1String ( "WhiteConsoleDockWidget" ) );
        m_wconsoleDock->setFeatures ( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable ); 
        addDockWidget ( Qt::LeftDockWidgetArea, m_wconsoleDock );
       
        m_chatDock = new QDockWidget ();
        m_chatDock->setObjectName ( QLatin1String ( "ChatDockWidget" ) );
        m_chatDock->setFeatures ( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );        
        addDockWidget ( Qt::LeftDockWidgetArea, m_chatDock );
    }    
    
    void MainWindow::setupHistoryDock()
    {
        m_historyDock = new QDockWidget ();
        m_historyDock->setObjectName ( QLatin1String("HistoryDockWidget") );
        m_historyDock->setFeatures ( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );
        
        m_historyDock->setWindowTitle ( i18n("Move History") );
        m_historyDock->setWidget ( new HistoryWidget );

        addDockWidget ( Qt::LeftDockWidgetArea, m_historyDock );
    }

    
    void MainWindow::protocolError ( Protocol::ErrorCode errorCode, const QString& errorString )
    {
        if ( errorCode != Protocol::UserCancelled )
        {
            KMessageBox::error ( this, errorString, Protocol::stringFromErrorCode ( errorCode ) );
        }
        Protocol::white()->deleteLater();
        Protocol::black()->deleteLater();
        //fileNew();
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
        
        dialog->addPage ( generalSettingsDlg, i18n ( "General" ), QLatin1String ( "games-config-options" ) );
        connect ( dialog, SIGNAL (settingsChanged(QString)), m_view, SLOT (settingsChanged()) );
        
        EngineSettings* engineSettings = new EngineSettings ( this );
        dialog->addPage ( engineSettings, i18n("Computer Engines"), QLatin1String("computer") );
        connect ( dialog, SIGNAL(accepted()), engineSettings, SLOT(writeConfig()) );
        
        QWidget* accessDlg = new QWidget;
        ui_prefs_access.setupUi ( accessDlg );
        dialog->addPage ( accessDlg, i18n( "Accessibility"), QLatin1String("preferences-desktop-accessibility") );
        
        QWidget* themeDlg = new KgThemeSelector ( m_themeProvider, KgThemeSelector::EnableNewStuffDownload, dialog );
        dialog->addPage ( themeDlg, i18n ( "Theme" ), QLatin1String ( "games-config-theme" ) );
        dialog->setAttribute ( Qt::WA_DeleteOnClose );
                
        dialog->show();
    }

    void MainWindow::pauseGame ( bool pause )
    {
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
        Settings::setShowClock( value );
    }
    
    void MainWindow::setShowHistorySetting(bool value)
    {
        Settings::setShowHistory ( value );
    }


    void MainWindow::setShowConsoleSetting()
    {
         if ( (actionCollection()->action( QLatin1String ( "show_console_white" ) )->isChecked() ) && (actionCollection()->action( QLatin1String ( "show_console_white" ) )->isVisible() ) )
         {
             Settings::setShowConsole( true );
         }
         else if ( (actionCollection()->action( QLatin1String ( "show_console_black" ) )->isChecked() ) && (actionCollection()->action( QLatin1String ( "show_console_black" ) )->isVisible() ) )
         {
             Settings::setShowConsole( true );
         }
         else
         {
             Settings::setShowConsole( false );
         }
    }

    void MainWindow::setShowChatSetting( bool value )
    {
        Settings::setShowChat( value );
    }

    void MainWindow::exitKnights()
    {
        //This will close the gnuchess/crafty/whatever process if it's running.
        Manager::self()->reset();
        Settings::self()->writeConfig();
    }
    
    void MainWindow::updateCaption()
    {
        if (Protocol::white() && Protocol::black())
        {
            setCaption( i18n ( "%1 vs. %2", Protocol::white()->playerName(), Protocol::black()->playerName() ) );
        }
    }

    bool MainWindow::queryClose()
    {
        return maybeSave();
    }
    
    
bool MainWindow::maybeSave()
{
    if (!Manager::self()->isGameActive())
    {
        return true;
    }
    
    bool ask = Settings::askDiscard();
    if (!ask)
    {
        return true;
    }
    
    Settings::setDontAskInternal( QString() );

    QString msg = i18n("This will end your game.\n"
                    "Would you like to save the move history?" );
    int result = KMessageBox::warningYesNoCancel( QApplication::activeWindow(), msg, QString(),
                                                  KStandardGuiItem::save(), 
                                                  KStandardGuiItem::discard(), 
                                                  KStandardGuiItem::cancel(),
                                                  QLatin1String(DontAskDiscard) );
    
    KMessageBox::ButtonCode res;
    Settings::setAskDiscard ( KMessageBox::shouldBeShownYesNo ( QLatin1String(DontAskDiscard), res ) );
    
    if (result == KMessageBox::Yes)
    {
        fileSave();
    }
    
    return result != KMessageBox::Cancel;
}
    
void MainWindow::fileSave()
{
    if ( m_fileName.isEmpty() )
    {
        m_fileName = KFileDialog::getSaveFileName ( KUrl("kfiledialog://knights"), i18n("*.pgn | Portable game notation" ) );
    }
    
    if ( m_fileName.isEmpty() )
    {
        return;
    }
    
    Manager::self()->saveGameHistoryAs( m_fileName );
    
    setCaption ( m_fileName );
}

void MainWindow::fileSaveAs()
{
    QString fileName = KFileDialog::getSaveFileName ( KUrl("kfiledialog://knights"), i18n("*.pgn | Portable game notation" ) );
    if ( fileName.isEmpty() )
    {
        return;
    }
    
    m_fileName = fileName;
    Manager::self()->saveGameHistoryAs( m_fileName );    
    setCaption ( m_fileName );
}


}

#include "knights.moc"

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
