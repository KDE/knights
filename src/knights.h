/***************************************************************************
    File                 : knights.h
    Project              : Knights
    Description          : Main window of the application
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-1018 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2010-2012 Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef KNIGHTS_H
#define KNIGHTS_H

#include "ui_prefs_base.h"
#include "ui_prefs_access.h"
#include "proto/protocol.h"

#include <KGameDifficulty>
#include <KXmlGuiWindow>

class KToggleAction;
class KGameThemeProvider;

namespace Knights {
class Protocol;
class KnightsView;
class ClockWidget;

class MainWindow : public KXmlGuiWindow {
	Q_OBJECT

public:
	MainWindow();

public Q_SLOTS:
	void fileSave();

private Q_SLOTS:
	void fileNew();
	void fileLoad();
	void fileSaveAs();
	void resign();
	void undo();
	void redo();
	void gameChanged();
	void gameOver(Knights::Color);
	void optionsPreferences();
	void activePlayerChanged();

	void protocolInitSuccesful();
	void protocolError(Protocol::ErrorCode errorCode, const QString& errorString);

	void setShowClockSetting(bool);
	void setShowHistorySetting(bool value);
	void setShowConsoleSetting();
	void setShowChatSetting(bool);
	void updateCaption();

	void exitKnights();

protected:
	void closeEvent(QCloseEvent*) override;

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

	QAction* m_saveAction;
	QAction* m_saveAsAction;
	QAction* m_undoAction;
	QAction* m_redoAction;
	KToggleAction* m_pauseAction;
	QAction* m_resignAction;
	QAction* m_drawAction;
	QAction* m_adjournAction;
	KToggleAction* m_toolbarAction;
	KToggleAction* m_statusbarAction;
	QMap<QString, Protocol::Feature> m_protocolFeatures;
	KGameThemeProvider* m_themeProvider;

	QString m_loadFileName;
	QString m_fileName;
};
}

#endif // KNIGHTS_H
