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

#ifndef KNIGHTS_H
#define KNIGHTS_H

#include "ui_prefs_base.h"
#include "ui_prefs_access.h"
#include "proto/protocol.h"

#include <KgDifficulty>
#include <KXmlGuiWindow>

class KToggleAction;
class KgThemeProvider;

namespace Knights
{
class Protocol;
class KnightsView;
class ClockWidget;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT

public:
    MainWindow();

public Q_SLOTS:
    void fileSave();

private Q_SLOTS:
    void fileNew();
    void fileLoad();
    void fileSaveAs();
    void pauseGame(bool);
    void undo();
    void redo();
    void optionsPreferences();

    void protocolInitSuccesful();
    void protocolError(Protocol::ErrorCode errorCode, const QString& errorString);

    void setShowClockSetting(bool);
    void setShowHistorySetting(bool value);
    void setShowConsoleSetting();
    void setShowChatSetting(bool);
    void updateCaption();

    void exitKnights();

private:
    void setupDocks();
    void setupActions();
    bool maybeSave();

    Ui::prefs_base ui_prefs_base;
    Ui::prefs_access ui_prefs_access;
    KnightsView* m_view;
    QDockWidget* m_clockDock;
    ClockWidget* m_playerClock;
    QDockWidget* m_wconsoleDock;
    QDockWidget* m_bconsoleDock;
    QDockWidget* m_chatDock;
    QDockWidget* m_historyDock;

    KToggleAction* m_pauseAction;
    KToggleAction* m_toolbarAction;
    KToggleAction* m_statusbarAction;
    QMap<QByteArray, Protocol::Feature> m_protocolFeatures;
    KgThemeProvider* m_themeProvider;

    QString m_loadFileName;
    QString m_fileName;
};
}

#endif // KNIGHTS_H
