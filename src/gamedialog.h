/***************************************************************************
    File                 : gamedialog.cpp
    Project              : Knights
    Description          : Game Dialog
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2018 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2009-2011 by Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
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

class QNetworkConfigurationManager;

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

	explicit GameDialog ( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
	~GameDialog() override;

	void setupProtocols();
	void save();

private:
	Ui::GameDialog* ui;
	QNetworkConfigurationManager* m_networkManager;
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
