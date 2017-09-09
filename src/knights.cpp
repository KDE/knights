/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>
    Copyright 2016 Alexander Semke <alexander.semke@web.de>

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
#include "proto/localprotocol.h"
#include "rules/chessrules.h"
#include "gamemanager.h"
#include "knightsview.h"
#include "settings.h"
#include "gamedialog.h"
#include "clockwidget.h"
#include "historywidget.h"
#include "enginesettings.h"
#include "knightsdebug.h"

#include <KConfigDialog>
#include <KActionCollection>
#include <KStandardAction>
#include <KToggleAction>
#include <KLocalizedString>
#include <KMessageBox>
#include <KgThemeSelector>
#include <KStandardGameAction>

#include <QTimer>
#include <QDockWidget>
#include <QStatusBar>
#include <QFileDialog>

const char* DontAskDiscard = "dontAskInternal";

using namespace Knights;

/**
* This class serves as the main window for Knights.  It handles the
* menus, toolbars, and status bars.
*
* @short Main window class
* @author %{AUTHOR} <%{EMAIL}>
* @version %{VERSION}
*/
MainWindow::MainWindow() : KXmlGuiWindow(),
    m_view(new KnightsView(this)),
    m_themeProvider(new KgThemeProvider("Theme", this))
{
    // accept dnd
    setAcceptDrops(true);

    // tell the KXmlGuiWindow that this is indeed the main widget
    setCentralWidget(m_view);

    // initial creation/setup of the docks
    setDockNestingEnabled(true);
    setupDocks();

    // setup actions and GUI
    setupActions();
    setupGUI();

	//protocol features
	m_protocolFeatures [ KStandardGameAction::name(KStandardGameAction::Pause) ] = Protocol::Pause;
    m_protocolFeatures [ "propose_draw" ] = Protocol::Draw;
    m_protocolFeatures [ "adjourn" ] = Protocol::Adjourn;
    m_protocolFeatures [ "resign" ] = Protocol::Resign;
    m_protocolFeatures [ "abort" ] = Protocol::Abort;

    // setup difficulty management
    connect(Kg::difficulty(), &KgDifficulty::currentLevelChanged, Manager::self(), &Manager::levelChanged);
    Kg::difficulty()->addLevel(new KgDifficultyLevel(0, "custom", i18n("Custom"), false));
    Kg::difficulty()->addStandardLevelRange(KgDifficultyLevel::VeryEasy, KgDifficultyLevel::VeryHard, KgDifficultyLevel::Medium);
    KgDifficultyGUI::init(this);

    // make all the docks invisible.
    // Show required docks after the game protocols are selected
    m_clockDock->hide();
    m_bconsoleDock->hide();
    m_wconsoleDock->hide();
    m_chatDock->hide();
	m_historyDock->hide();
    statusBar()->hide();

    connect(m_view, &KnightsView::gameNew, this, &MainWindow::fileNew, Qt::QueuedConnection);
    connect(Manager::self(), &Manager::initComplete, this, &MainWindow::protocolInitSuccesful);
    connect(Manager::self(), &Manager::playerNameChanged, this, &MainWindow::updateCaption);
	connect(Manager::self(), &Manager::pieceMoved, this, &MainWindow::gameChanged);
    connect(qApp, &QGuiApplication::lastWindowClosed, this, &MainWindow::exitKnights);

	Manager::self()->setRules(new ChessRules);
	m_themeProvider->discoverThemes("appdata", QLatin1String("themes"));
	m_view->drawBoard(m_themeProvider);
}

void MainWindow::setupDocks()
{
    // clock dock
    m_clockDock = new QDockWidget(i18n("Clock"), this);
    m_clockDock->setObjectName(QLatin1String("ClockDockWidget"));       // for QMainWindow::saveState()
    m_playerClock = new ClockWidget(this);
    m_clockDock->setWidget(m_playerClock);
    m_clockDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    connect(m_view, &KnightsView::displayedPlayerChanged, m_playerClock, &ClockWidget::setDisplayedPlayer);
    connect(Manager::self(), &Manager::timeChanged, m_playerClock, &ClockWidget::setCurrentTime);
    addDockWidget(Qt::RightDockWidgetArea, m_clockDock);

    // console dock for black
    m_bconsoleDock = new QDockWidget();
    m_bconsoleDock->setObjectName(QLatin1String("BlackConsoleDockWidget"));
    m_bconsoleDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, m_bconsoleDock);

    // console dock for white
    m_wconsoleDock = new QDockWidget();
    m_wconsoleDock->setObjectName(QLatin1String("WhiteConsoleDockWidget"));
    m_wconsoleDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, m_wconsoleDock);

    // chat dock
    m_chatDock = new QDockWidget();
    m_chatDock->setObjectName(QLatin1String("ChatDockWidget"));
    m_chatDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    addDockWidget(Qt::LeftDockWidgetArea, m_chatDock);

    // history dock
    m_historyDock = new QDockWidget();
    m_historyDock->setObjectName(QLatin1String("HistoryDockWidget"));
    m_historyDock->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
    m_historyDock->setWindowTitle(i18n("Move History"));
    m_historyDock->setWidget(new HistoryWidget);
    addDockWidget(Qt::LeftDockWidgetArea, m_historyDock);
}

void MainWindow::setupActions()
{
    KStandardGameAction::gameNew(this, SLOT(fileNew()), actionCollection());
    KStandardGameAction::quit(qApp, SLOT(closeAllWindows()), actionCollection());
    m_pauseAction = KStandardGameAction::pause(this, SLOT(pauseGame(bool)), actionCollection());
    m_pauseAction->setEnabled(false);
    KStandardAction::preferences(this, SLOT(optionsPreferences()), actionCollection());

    m_saveAction = KStandardGameAction::save(this, SLOT(fileSave()), actionCollection());
	m_saveAction->setEnabled(false);
    m_saveAsAction = KStandardGameAction::saveAs(this, SLOT(fileSaveAs()), actionCollection());
	m_saveAsAction->setEnabled(false);
    KStandardGameAction::load(this, SLOT(fileLoad()), actionCollection());

    QAction* resignAction = actionCollection()->addAction(QLatin1String("resign"), Manager::self(), SLOT(resign()));
    resignAction->setText(i18n("Resign"));
    resignAction->setToolTip(i18n("Admit your inevitable defeat"));
    resignAction->setIcon(QIcon::fromTheme(QLatin1String("flag-red")));
    resignAction->setEnabled(false);

    QAction* undoAction = actionCollection()->addAction(QLatin1String("move_undo"), this, SLOT(undo()));
    undoAction->setText(i18n("Undo"));
    undoAction->setToolTip(i18n("Take back your last move"));
    undoAction->setIcon(QIcon::fromTheme(QLatin1String("edit-undo")));
    connect(Manager::self(), &Manager::undoPossible, undoAction, &QAction::setEnabled);
    undoAction->setEnabled(false);

    QAction* redoAction = actionCollection()->addAction(QLatin1String("move_redo"), this, SLOT(redo()));
    redoAction->setText(i18n("Redo"));
    redoAction->setToolTip(i18n("Repeat your last move"));
    redoAction->setIcon(QIcon::fromTheme(QLatin1String("edit-redo")));
    connect(Manager::self(), &Manager::redoPossible, redoAction, &QAction::setEnabled);
    redoAction->setEnabled(false);

    QAction* drawAction = actionCollection()->addAction(QLatin1String("propose_draw"), Manager::self(), SLOT(offerDraw()));
    drawAction->setText(i18n("Offer &Draw"));
    drawAction->setToolTip(i18n("Offer a draw to your opponent"));
    drawAction->setIcon(QIcon::fromTheme(QLatin1String("flag-blue")));
    drawAction->setEnabled(false);

    QAction* adjournAction = actionCollection()->addAction(QLatin1String("adjourn"), Manager::self(), SLOT(adjourn()));
    adjournAction->setText(i18n("Adjourn"));
    adjournAction->setToolTip(i18n("Continue this game at a later time"));
    adjournAction->setIcon(QIcon::fromTheme(QLatin1String("document-save")));
    adjournAction->setEnabled(false);

    QAction* abortAction = actionCollection()->addAction(QLatin1String("abort"), Manager::self(), SLOT(abort()));
    abortAction->setText(i18n("Abort"));
    abortAction->setToolTip(i18n("End the game immediately"));
    abortAction->setIcon(QIcon::fromTheme(QLatin1String("dialog-cancel")));
    abortAction->setEnabled(false);

    KToggleAction* clockAction = new KToggleAction(QIcon::fromTheme(QLatin1String("clock")), i18n("Show Clock"), actionCollection());
    actionCollection()->addAction(QLatin1String("show_clock"), clockAction);
    connect(clockAction, &KToggleAction::triggered, m_clockDock, &QDockWidget::setVisible);
    connect(clockAction, &KToggleAction::triggered, this, &MainWindow::setShowClockSetting);
    clockAction->setVisible(false);

    KToggleAction* historyAction = new KToggleAction(QIcon::fromTheme(QLatin1String("view-history")), i18n("Show History"), actionCollection());
    actionCollection()->addAction(QLatin1String("show_history"), historyAction);
    connect(historyAction, &KToggleAction::triggered, m_historyDock, &QDockWidget::setVisible);
    connect(historyAction, &KToggleAction::triggered, this, &MainWindow::setShowHistorySetting);
    historyAction->setVisible(true);
    historyAction->setChecked(Settings::showHistory());

    KToggleAction* wconsoleAction = new KToggleAction(QIcon::fromTheme(QLatin1String("utilities-terminal")), i18n("Show White Console"), actionCollection());
    actionCollection()->addAction(QLatin1String("show_console_white"), wconsoleAction);
    connect(wconsoleAction, &KToggleAction::triggered, m_wconsoleDock, &QDockWidget::setVisible);
    connect(wconsoleAction, &KToggleAction::triggered, this, &MainWindow::setShowConsoleSetting);
    wconsoleAction->setVisible(false);

    KToggleAction* bconsoleAction = new KToggleAction(QIcon::fromTheme(QLatin1String("utilities-terminal")), i18n("Show Black Console"), actionCollection());
    actionCollection()->addAction(QLatin1String("show_console_black"), bconsoleAction);
    connect(bconsoleAction, &KToggleAction::triggered, m_bconsoleDock, &QDockWidget::setVisible);
    connect(bconsoleAction, &KToggleAction::triggered, this, &MainWindow::setShowConsoleSetting);
    bconsoleAction->setVisible(false);

    KToggleAction* chatAction = new KToggleAction(QIcon::fromTheme(QLatin1String("meeting-attending")), i18n("Show Chat"), actionCollection());
    actionCollection()->addAction(QLatin1String("show_chat"), chatAction);
    connect(chatAction, &KToggleAction::triggered, m_chatDock, &QDockWidget::setVisible);
    connect(chatAction, &KToggleAction::triggered, this, &MainWindow::setShowChatSetting);
    chatAction->setVisible(false);
}

void MainWindow::fileNew()
{
    if(!maybeSave())
    {
        return;
    }

    GameDialog* gameNewDialog = new GameDialog(this);
    if(gameNewDialog->exec() == QDialog::Accepted)
    {
        Manager::self()->reset();
        gameNewDialog->setupProtocols();
        connect(Protocol::white(), &Protocol::error, this, &MainWindow::protocolError);
        connect(Protocol::black(), &Protocol::error, this, &MainWindow::protocolError);
        gameNewDialog->save();

        m_pauseAction->setChecked(false);
        Manager::self()->initialize();

        if ( (Protocol::white() && Protocol::white()->supportedFeatures()& Protocol::AdjustDifficulty)
                || (Protocol::white() && Protocol::white()->supportedFeatures()& Protocol::AdjustDifficulty) )
            statusBar()->show();
        else
            statusBar()->hide();
    }
    delete gameNewDialog;

	m_saveAction->setEnabled(false);
	m_saveAsAction->setEnabled(false);
}

void MainWindow::fileLoad()
{
    if(!maybeSave())
    {
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(this, i18n("Open File"),
                       QStandardPaths::displayName(QStandardPaths::DocumentsLocation),
                       i18n("Portable game notation (*.pgn)"));
    if(fileName.isEmpty())
    {
        return;
    }

    Manager::self()->reset();
    Protocol::setWhiteProtocol(new LocalProtocol());
    Protocol::setBlackProtocol(new LocalProtocol());

    connect(Protocol::white(), &Protocol::error, this, &MainWindow::protocolError);
    connect(Protocol::black(), &Protocol::error, this, &MainWindow::protocolError);

    m_loadFileName = fileName;
    Manager::self()->initialize();
	m_saveAction->setEnabled(false);
}

//TODO
// void MainWindow::showFicsDialog(Color color, bool computer)
// {
//     if(computer || true)   // TODO: Implement, and remove this check
//     {
//         KMessageBox::sorry(this, i18n("This feature is not yet implemented in Knights"));
//         QTimer::singleShot(1, this, SLOT(fileNew()));
//         return;
//     }
//     FicsProtocol* p = new FicsProtocol();
//     p->setColor(color);
//     p->init();
//     p->openGameDialog();
// }

void MainWindow::protocolInitSuccesful()
{
    qCDebug(LOG_KNIGHTS) << "Show Clock:" << Settings::showClock() << "Show Console:" << Settings::showConsole();
    QString whiteName = Protocol::white()->playerName();
    QString blackName = Protocol::black()->playerName();
    updateCaption();

    // show clock action button if timer active
    // show clock dock widget if timer active and configuration file entry has visible = true
    if(Manager::self()->timeControlEnabled(White) || Manager::self()->timeControlEnabled(Black))
    {
        actionCollection()->action(QLatin1String("show_clock"))->setVisible(true);
        m_playerClock->setPlayerName(White, Protocol::white()->playerName());
        m_playerClock->setPlayerName(Black, Protocol::black()->playerName());
        m_playerClock->setTimeLimit(White, Manager::self()->timeLimit(White));
        m_playerClock->setTimeLimit(Black, Manager::self()->timeLimit(Black));
        if(Settings::showClock())
        {
            m_clockDock->setVisible(true);
            actionCollection()->action(QLatin1String("show_clock"))->setChecked(true);
        }
        else
        {
            m_clockDock->setVisible(false);
            actionCollection()->action(QLatin1String("show_clock"))->setChecked(false);
        }
    }
    else
    {
        m_clockDock->setVisible(false);
        actionCollection()->action(QLatin1String("show_clock"))->setVisible(false);
    }

    Protocol::Features f = Protocol::NoFeatures;
    if(Protocol::white()->isLocal() && !(Protocol::black()->isLocal()))
    {
        f = Protocol::black()->supportedFeatures();
    }
    else if(Protocol::black()->isLocal() && !(Protocol::white()->isLocal()))
    {
        f = Protocol::white()->supportedFeatures();
    }
    else if(!(Protocol::black()->isLocal()) && !(Protocol::white()->isLocal()))
    {
        // These protocol features make sense when neither player is local
        f = Protocol::Pause | Protocol::Adjourn | Protocol::Abort;
        f &= Protocol::white()->supportedFeatures();
        f &= Protocol::black()->supportedFeatures();
    }

    QMap<QByteArray, Protocol::Feature>::ConstIterator it;
    for(it = m_protocolFeatures.constBegin(); it != m_protocolFeatures.constEnd(); ++it)
    {
        actionCollection()->action(QLatin1String(it.key()))->setEnabled(f & it.value());
    }

    // show console action button if protocol allows a console
    // show console dock widget if protocol allows and configuration file entry has visible = true
    // finally, hide any dock widget not needed - in case it is still active from previous game
    actionCollection()->action(QLatin1String("show_console_white"))->setVisible(false);
    actionCollection()->action(QLatin1String("show_console_black"))->setVisible(false);
    actionCollection()->action(QLatin1String("show_chat"))->setVisible(false);
    QList<Protocol::ToolWidgetData> list;
    list << Protocol::black()->toolWidgets();
    list << Protocol::white()->toolWidgets();
    foreach(const Protocol::ToolWidgetData& data, list)
    {
        switch(data.type)
        {
        case Protocol::ConsoleToolWidget:
            if(data.owner == White)
            {
                m_wconsoleDock->setWindowTitle(data.title);
                m_wconsoleDock->setWidget(data.widget);
                actionCollection()->action(QLatin1String("show_console_white"))->setVisible(true);
                if(Settings::showConsole())
                {
                    m_wconsoleDock->setVisible(true);
                    actionCollection()->action(QLatin1String("show_console_white"))->setChecked(true);
                }
                else
                {
                    m_wconsoleDock->setVisible(false);
                    actionCollection()->action(QLatin1String("show_console_white"))->setChecked(false);
                }
            }
            else
            {
                m_bconsoleDock->setWindowTitle(data.title);
                m_bconsoleDock->setWidget(data.widget);
                actionCollection()->action(QLatin1String("show_console_black"))->setVisible(true);
                if(Settings::showConsole())
                {
                    m_bconsoleDock->setVisible(true);
                    actionCollection()->action(QLatin1String("show_console_black"))->setChecked(true);
                }
                else
                {
                    m_bconsoleDock->setVisible(false);
                    actionCollection()->action(QLatin1String("show_console_black"))->setChecked(false);
                }
            }
            break;

        case Protocol::ChatToolWidget:
            m_chatDock->setWindowTitle(data.title);
            m_chatDock->setWidget(data.widget);
            actionCollection()->action(QLatin1String("show_chat"))->setVisible(true);
            if(Settings::showChat())
            {
                m_chatDock->setVisible(true);
                actionCollection()->action(QLatin1String("show_chat"))->setChecked(true);
            }
            else
            {
                m_chatDock->setVisible(false);
                actionCollection()->action(QLatin1String("show_chat"))->setChecked(false);
            }
            break;

        default:
            break;
        }
    }
    if(!actionCollection()->action(QLatin1String("show_console_white"))->isVisible())
    {
        m_wconsoleDock->hide();
    }
    if(!actionCollection()->action(QLatin1String("show_console_black"))->isVisible())
    {
        m_bconsoleDock->hide();
    }
    if(!actionCollection()->action(QLatin1String("show_chat"))->isVisible())
    {
        m_chatDock->hide();
    }

    Manager::self()->setRules(new ChessRules);
    Manager::self()->startGame();

    if(m_loadFileName.isEmpty())
    {
        m_view->setupBoard(m_themeProvider);
    }
    else
    {
        int speed = Settings::animationSpeed();
        Settings::setAnimationSpeed(Settings::EnumAnimationSpeed::Instant);
        m_view->setupBoard(m_themeProvider);

        Manager::self()->loadGameHistoryFrom(m_loadFileName);
        setCaption(m_loadFileName);

        m_loadFileName.clear();
        Settings::setAnimationSpeed(speed);
    }
}

void MainWindow::protocolError(Protocol::ErrorCode errorCode, const QString& errorString)
{
    if(errorCode != Protocol::UserCancelled)
    {
        KMessageBox::error(this, errorString, Protocol::stringFromErrorCode(errorCode));
    }
    Protocol::white()->deleteLater();
    Protocol::black()->deleteLater();
    //fileNew();
}

void MainWindow::optionsPreferences()
{
    if(KConfigDialog::showDialog(QLatin1String("settings")))
    {
        return;
    }
    KConfigDialog *dialog = new KConfigDialog(this, QLatin1String("settings"), Settings::self());
    QWidget *generalSettingsDlg = new QWidget;
    ui_prefs_base.setupUi(generalSettingsDlg);

    dialog->addPage(generalSettingsDlg, i18n("General"), QLatin1String("games-config-options"));
    connect(dialog, &KConfigDialog::settingsChanged, m_view, &KnightsView::settingsChanged);

    EngineSettings* engineSettings = new EngineSettings(this);
    dialog->addPage(engineSettings, i18n("Computer Engines"), QLatin1String("computer"));
    connect(dialog, &KConfigDialog::accepted, engineSettings, &EngineSettings::save);

    QWidget* accessDlg = new QWidget;
    ui_prefs_access.setupUi(accessDlg);
    dialog->addPage(accessDlg, i18n("Accessibility"), QLatin1String("preferences-desktop-accessibility"));

    QWidget* themeDlg = new KgThemeSelector(m_themeProvider, KgThemeSelector::EnableNewStuffDownload, dialog);
    dialog->addPage(themeDlg, i18n("Theme"), QLatin1String("games-config-theme"));
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    dialog->show();
}

void MainWindow::pauseGame(bool pause)
{
    Offer o;
    o.action = pause ? ActionPause : ActionResume;
    o.id = qrand();
    o.player = NoColor;
    Manager::self()->sendOffer(o);
}

void MainWindow::undo()
{
    if(!Protocol::white()->isLocal() && !Protocol::black()->isLocal())
    {
        // No need to pause the game if both players are local
        QAction* pa = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));
        if(pa)
        {
            pa->setChecked(true);
        }
    }
    Manager::self()->undo();
}

void MainWindow::redo()
{
    Manager::self()->redo();
    if(!Protocol::white()->isLocal() && !Protocol::black()->isLocal())
    {
        // No need to pause the game if both players are local
        QAction* pa = actionCollection()->action(QLatin1String(KStandardGameAction::name(KStandardGameAction::Pause)));
        if(pa && !Manager::self()->canRedo())
        {
            pa->setChecked(false);
        }
    }
}

void MainWindow::gameChanged() {
	m_saveAction->setEnabled(true);
	m_saveAsAction->setEnabled(true);
}

void MainWindow::setShowClockSetting(bool value)
{
    Settings::setShowClock(value);
}

void MainWindow::setShowHistorySetting(bool value)
{
    Settings::setShowHistory(value);
}


void MainWindow::setShowConsoleSetting()
{
    if((actionCollection()->action(QLatin1String("show_console_white"))->isChecked()) && (actionCollection()->action(QLatin1String("show_console_white"))->isVisible()))
    {
        Settings::setShowConsole(true);
    }
    else if((actionCollection()->action(QLatin1String("show_console_black"))->isChecked()) && (actionCollection()->action(QLatin1String("show_console_black"))->isVisible()))
    {
        Settings::setShowConsole(true);
    }
    else
    {
        Settings::setShowConsole(false);
    }
}

void MainWindow::setShowChatSetting(bool value)
{
    Settings::setShowChat(value);
}

void MainWindow::exitKnights()
{
    //This will close the gnuchess/crafty/whatever process if it's running.
    Manager::self()->reset();
    Settings::self()->save();
}

void MainWindow::updateCaption()
{
    if(Protocol::white() && Protocol::black())
    {
        setCaption(i18n("%1 vs. %2", Protocol::white()->playerName(), Protocol::black()->playerName()));
    }
}


bool MainWindow::maybeSave()
{
    if(!Manager::self()->isGameActive())
    {
        return true;
    }

    bool ask = Settings::askDiscard();
    if(!ask)
    {
        return true;
    }

    Settings::setDontAskInternal(QString());

    QString msg = i18n("This will end your game.\n"
                       "Would you like to save the move history?");
    int result = KMessageBox::warningYesNoCancel(QApplication::activeWindow(), msg, QString(),
                 KStandardGuiItem::save(),
                 KStandardGuiItem::discard(),
                 KStandardGuiItem::cancel(),
                 QLatin1String(DontAskDiscard));

    KMessageBox::ButtonCode res;
    Settings::setAskDiscard(KMessageBox::shouldBeShownYesNo(QLatin1String(DontAskDiscard), res));

    if(result == KMessageBox::Yes)
    {
        fileSave();
    }

    return result != KMessageBox::Cancel;
}

void MainWindow::fileSave()
{
    if(m_fileName.isEmpty())
    {
        m_fileName = QFileDialog::getSaveFileName(this, i18n("Save"), QStandardPaths::displayName(QStandardPaths::DocumentsLocation),
                     i18n("Portable game notation (*.pgn)"));
    }

    if(m_fileName.isEmpty())
    {
        return;
    }

    Manager::self()->saveGameHistoryAs(m_fileName);

    setCaption(m_fileName);
	m_saveAction->setEnabled(false);
}

void MainWindow::fileSaveAs()
{
    QString fileName = QFileDialog::getSaveFileName(this, i18n("Save As"), QStandardPaths::displayName(QStandardPaths::DocumentsLocation),
                       i18n("Portable game notation (*.pgn)"));
    if(fileName.isEmpty())
    {
        return;
    }

    m_fileName = fileName;
    Manager::self()->saveGameHistoryAs(m_fileName);
    setCaption(m_fileName);
}
