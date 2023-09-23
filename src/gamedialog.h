/***************************************************************************
    File                 : gamedialog.cpp
    Project              : Knights
    Description          : Game Dialog
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2018 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2009-2011 Miha Čančula (miha@noughmad.eu)

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
#ifndef KNIGHTS_GAMEDIALOG_H
#define KNIGHTS_GAMEDIALOG_H

#include <QDialog>

namespace Ui {
class GameDialog;
}

namespace Knights {

class GameDialog : public QDialog {
	Q_OBJECT

public:
	enum FicsMode {
		NoFics,
		PlayerVsFics,
		ComputerVsFics,
		BothPlayersFics
	};

    explicit GameDialog ( QWidget* parent = nullptr, Qt::WindowFlags f = {} );
	~GameDialog() override;

	void setupProtocols();
	void save();

private:
	Ui::GameDialog* ui;
	QPushButton* okButton;

private Q_SLOTS:
	void player1SettingsChanged();
	void player2SettingsChanged();
	void timeControlChanged();
	void updateTimeEdits();
	void networkStatusChanged(bool isOnline);
	void loadEngines();
	void showEngineConfigDialog();
	void checkOkButton();
};

}

#endif // KNIGHTS_GAMEDIALOG_H
