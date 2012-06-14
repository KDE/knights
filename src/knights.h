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

#ifndef KNIGHTS_H
#define KNIGHTS_H

#include "ui_prefs_base.h"
#include "ui_prefs_access.h"
#include "proto/protocol.h"
#include <KgDifficulty>

#include <KXmlGuiWindow>

class QStringListModel;
class KToggleAction;
class KgThemeProvider;

namespace Knights
{
    class Protocol;
    class KnightsView;
    class ClockWidget;

    /**
    * This class serves as the main window for Knights.  It handles the
    * menus, toolbars, and status bars.
    *
    * @short Main window class
    * @author %{AUTHOR} <%{EMAIL}>
    * @version %{VERSION}
    */
    class MainWindow : public KXmlGuiWindow
    {
            Q_OBJECT
        public:
            MainWindow();
            virtual ~MainWindow();
            virtual bool queryClose();
            bool maybeSave();

        public Q_SLOTS:
            void fileNew();
            void fileLoad();
            void fileSave();
            void fileSaveAs();
            void pauseGame ( bool pause );
            void undo();
            void redo();
            void optionsPreferences();
            
        private Q_SLOTS:

            void protocolInitSuccesful();
            void protocolError ( Protocol::ErrorCode errorCode, const QString& errorString );

            void setShowClockSetting( bool value );
            void setShowHistorySetting( bool value );
            void setShowConsoleSetting();
            void setShowChatSetting( bool value );
            void updateCaption();

            void exitKnights();

        private:
            void setupActions();
            void setupClockDock();
            void setupConsoleDocks();
            void setupHistoryDock();
            
            void showFicsDialog( Color color = NoColor, bool computer = false);
            void showFicsSpectateDialog();

        private:
            Ui::prefs_base ui_prefs_base;
            Ui::prefs_access ui_prefs_access;
            KnightsView *m_view;
            QPointer<QDockWidget> m_clockDock;
            QPointer<ClockWidget> playerClock;
            QPointer<QDockWidget> m_wconsoleDock;
            QPointer<QDockWidget> m_bconsoleDock;
            QPointer<QDockWidget> m_chatDock;
            QPointer<QDockWidget> m_historyDock;

            KToggleAction * pause;

            KToggleAction *m_toolbarAction;
            KToggleAction *m_statusbarAction;
            QMap<QByteArray, Protocol::Feature> protocolFeatures;
            KgThemeProvider* m_themeProvider;
            
            QString m_loadFileName;
    QString m_fileName;
    };
}

#endif // KNIGHTS_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
