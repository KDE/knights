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

#include "gamedialog.h"

#include "ui_gamedialog.h"
#include "proto/localprotocol.h"
#include "proto/xboardprotocol.h"
#include "proto/ficsprotocol.h"
#include "proto/uciprotocol.h"
#include "gamemanager.h"
#include "knightsdebug.h"
#include "settings.h"
#include "enginesettings.h"

#include <QNetworkConfigurationManager>
#include <QDialogButtonBox>

using namespace Knights;

GameDialog::GameDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f),
	ui(new Ui::GameDialog()),
	m_networkManager(new QNetworkConfigurationManager(this)),
	okButton(nullptr) {

	QFrame* mainFrame = new QFrame(this);
	ui->setupUi(mainFrame);
	ui->pbPlayer1Engine->setIcon(QIcon::fromTheme(QLatin1String("configure")));
	ui->pbPlayer2Engine->setIcon(QIcon::fromTheme(QLatin1String("configure")));
	ui->cbTimeControl->setChecked(Settings::timeEnabled());
	ui->sbTimeLimit->setValue(Settings::timeLimit());
	ui->sbTimeIncrement->setValue(Settings::timeIncrement());
	ui->sbNumberOfMoves->setValue(Settings::numberOfMoves());

	QDialogButtonBox* bBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
	okButton = bBox->button(QDialogButtonBox::Ok);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mainFrame);
	layout->addWidget(bBox);

	setLayout(layout);
	setWindowTitle(i18n("New Game"));

	connect(bBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(bBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

	switch(Settings::player1Protocol()) {
	case Settings::EnumPlayer1Protocol::Local:
		ui->rbPlayer1Human->setChecked(true);
		break;
	case Settings::EnumPlayer1Protocol::XBoard:
		ui->rbPlayer1Engine->setChecked(true);
		break;
	}

	switch(Settings::color()) {
	case Settings::EnumColor::NoColor:
		ui->colorRandom->setChecked(true);
		break;
	case Settings::EnumColor::White:
		ui->colorWhite->setChecked(true);
		break;
	case Settings::EnumColor::Black:
		ui->colorBlack->setChecked(true);
		break;
	}

	switch(Settings::player2Protocol()) {
	case Settings::EnumPlayer2Protocol::Local:
		ui->rbPlayer2Human->setChecked(true);
		break;
	case Settings::EnumPlayer2Protocol::XBoard:
		ui->rbPlayer2Engine->setChecked(true);
		break;
	case Settings::EnumPlayer2Protocol::Fics:
		ui->rbPlayer2Server->setChecked(true);
		break;
	}

	loadEngines();

	ui->cbPlayer2Server->setHistoryItems(Settings::servers());
	ui->cbPlayer2Server->setEditText(Settings::currentServer());

	//SIGNALs/SLOTs
	//player 1
	connect(ui->rbPlayer1Human, &QRadioButton::clicked, this, &GameDialog::player1SettingsChanged);
	connect(ui->rbPlayer1Engine, &QRadioButton::clicked, this, &GameDialog::player1SettingsChanged);
	connect(ui->pbPlayer1Engine, &QPushButton::clicked, this, &GameDialog::showEngineConfigDialog);
	connect(ui->cbPlayer1Engine, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GameDialog::checkOkButton);

	//player 2
	connect(ui->rbPlayer2Human, &QRadioButton::clicked, this, &GameDialog::player2SettingsChanged);
	connect(ui->rbPlayer2Engine, &QRadioButton::clicked, this, &GameDialog::player2SettingsChanged);
	connect(ui->rbPlayer2Server, &QRadioButton::clicked, this, &GameDialog::player2SettingsChanged);
	connect(ui->pbPlayer2Engine, &QPushButton::clicked, this, &GameDialog::showEngineConfigDialog);
	connect(ui->cbPlayer2Engine, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &GameDialog::checkOkButton);

	//time control
	connect(ui->cbTimeControl, &QCheckBox::toggled, this, &GameDialog::timeControlChanged);
	connect(ui->sbTimeLimit, static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged), this, &GameDialog::updateTimeEdits);
	connect(ui->sbTimeIncrement, static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged), this, &GameDialog::updateTimeEdits);
	connect(ui->sbNumberOfMoves, static_cast<void (QSpinBox::*)(int)> (&QSpinBox::valueChanged), this, &GameDialog::updateTimeEdits);

	connect(m_networkManager, &QNetworkConfigurationManager::onlineStateChanged, this, &GameDialog::networkStatusChanged);

	networkStatusChanged(m_networkManager->isOnline());
	updateTimeEdits();
	player1SettingsChanged();
	player2SettingsChanged();
	timeControlChanged();
}

GameDialog::~GameDialog() {
	delete ui;
}

void GameDialog::setupProtocols() {
	TimeControl tc;
	tc.baseTime = ui->cbTimeControl->isChecked() ? QTime(0, ui->sbTimeLimit->value(), 0, 0) : QTime();
	tc.moves = ui->rbPlayer2Server->isChecked() ? 0 : ui->sbNumberOfMoves->value();
	tc.increment = ui->sbTimeIncrement->value();
	Manager::self()->setTimeControl(NoColor, tc);

	QList<EngineConfiguration> configs;
	for (const QString& s : Settings::engineConfigurations())
		configs << EngineConfiguration(s);

	Protocol* p1 = nullptr;
	Protocol* p2 = nullptr;
	Color c1 = NoColor;
	if(ui->rbPlayer1Human->isChecked())
		p1 = new LocalProtocol;
	else {
		if(ui->cbPlayer2Engine->currentIndex() != -1) {
			EngineConfiguration c = configs [ ui->cbPlayer1Engine->currentIndex() ];
			if(c.iface == EngineConfiguration::XBoard)
				p1 = new XBoardProtocol;
			else
				p1 = new UciProtocol;
			p1->setAttribute("program", c.commandLine);
			p1->setPlayerName(c.name);
		} else {
			//This happens when the user didn't select any game engine, so we are
			//creating below a dummy XBoardProtocol object with an invalid command
			//path (i.e. an empty QString), thus this object will send a signal
			//error which will end up in a notification of the error to the user,
			//raising a KMessageBox error or so.
			p1 = new XBoardProtocol;
			p1->setAttribute("program", QString());
		}
	}

	if(ui->colorWhite->isChecked())
		c1 = White;
	else if(ui->colorBlack->isChecked())
		c1 = Black;

	if(ui->rbPlayer2Human->isChecked())
		p2 = new LocalProtocol;
	else if(ui->rbPlayer2Engine->isChecked()) {
		if(ui->cbPlayer2Engine->currentIndex() != -1) {
			EngineConfiguration c = configs [ ui->cbPlayer2Engine->currentIndex() ];
			if(c.iface == EngineConfiguration::XBoard)
				p2 = new XBoardProtocol;
			else
				p2 = new UciProtocol;
			p2->setAttribute("program", c.commandLine);
			p2->setPlayerName(c.name);
		} else {
			//This happens when the user didn't select any game engine, so we are
			//creating below a dummy XBoardProtocol object with an invalid command
			//path (i.e. an empty QString), thus this object will send a signal
			//error which will end up in a notification of the error to the user,
			//raising a KMessageBox error or so.
			p2 = new XBoardProtocol;
			p2->setAttribute("program", QString());
		}
	} else {
		p2 = new FicsProtocol;
		p2->setAttribute("server", ui->cbPlayer2Server->currentText());
	}
	if(c1 == NoColor) {
		qsrand(QTime::currentTime().msec());
		c1 = (qrand() % 2) ? White : Black;
	}
	// Color-changing by the FICS protocol happens later, so it doesn't matter what we do here.
	Protocol::setWhiteProtocol(c1 == White ? p1 : p2);
	Protocol::setBlackProtocol(c1 == White ? p2 : p1);
}

void GameDialog::save() {
	Settings::EnumPlayer1Protocol::type p1;
	if(ui->rbPlayer1Human->isChecked())
		p1 = Settings::EnumPlayer1Protocol::Local;
	else {
		p1 = Settings::EnumPlayer1Protocol::XBoard;
		Settings::setPlayer1Program(ui->cbPlayer1Engine->currentText());
	}
	Settings::setPlayer1Protocol(p1);

	Settings::EnumPlayer2Protocol::type p2;
	if(ui->rbPlayer2Human->isChecked())
		p2 = Settings::EnumPlayer2Protocol::Local;
	else if(ui->rbPlayer2Engine->isChecked()) {
		p2 = Settings::EnumPlayer2Protocol::XBoard;
		Settings::setPlayer2Program(ui->cbPlayer2Engine->currentText());
	} else {
		p2 = Settings::EnumPlayer2Protocol::Fics;
		Settings::setServers(ui->cbPlayer2Server->historyItems());
		Settings::setCurrentServer(ui->cbPlayer2Server->currentText());
	}
	Settings::setPlayer2Protocol(p2);

	Settings::EnumColor::type selectedColor = Settings::EnumColor::NoColor;
	if(ui->colorWhite->isChecked())
		selectedColor = Settings::EnumColor::White;
	else if(ui->colorBlack->isChecked())
		selectedColor = Settings::EnumColor::Black;
	Settings::setColor(selectedColor);

	bool timeLimitEnabled = ui->cbTimeControl->isChecked();
	Settings::setTimeEnabled(timeLimitEnabled);
	if(timeLimitEnabled) {
		Settings::setTimeLimit(ui->sbTimeLimit->value());
		Settings::setTimeIncrement(ui->sbTimeIncrement->value());
		if(p2 != Settings::EnumPlayer2Protocol::Fics)
			Settings::setNumberOfMoves(ui->sbNumberOfMoves->value());
	}

	Settings::self()->save();
}

void GameDialog::updateTimeEdits() {
	ui->sbTimeLimit->setSuffix(i18np(" minute", " minutes", ui->sbTimeLimit->value()));
	ui->sbTimeIncrement->setSuffix(i18np(" second", " seconds", ui->sbTimeIncrement->value()));
	ui->sbNumberOfMoves->setSuffix(i18np(" move", " moves", ui->sbNumberOfMoves->value()));
}

void GameDialog::player1SettingsChanged() {
	ui->cbPlayer1Engine->setEnabled(ui->rbPlayer1Engine->isChecked());
	ui->pbPlayer1Engine->setEnabled(ui->rbPlayer1Engine->isChecked());
	checkOkButton();
}

void GameDialog::player2SettingsChanged() {
	ui->cbPlayer2Engine->setEnabled(ui->rbPlayer2Engine->isChecked());
	ui->pbPlayer2Engine->setEnabled(ui->rbPlayer2Engine->isChecked());
	ui->cbPlayer2Server->setEnabled(ui->rbPlayer2Server->isChecked());

	bool server = ui->rbPlayer2Server->isChecked();
	if(server)
		ui->cbTimeControl->setChecked(true);
	ui->cbTimeControl->setEnabled(!server);
	ui->lNumberOfMoves->setVisible(!server);
	ui->sbNumberOfMoves->setVisible(!server);
	checkOkButton();
}

void GameDialog::timeControlChanged() {
	bool b = ui->cbTimeControl->isChecked();
	ui->sbNumberOfMoves->setEnabled(b);
	ui->sbTimeLimit->setEnabled(b);
	ui->sbTimeIncrement->setEnabled(b);
}

void GameDialog::networkStatusChanged(bool isOnline) {
	qCDebug(LOG_KNIGHTS) << isOnline;
	if(!isOnline && ui->rbPlayer2Server->isChecked())
		ui->rbPlayer2Engine->setChecked(true);
	ui->rbPlayer2Server->setEnabled(isOnline);
}

void GameDialog::showEngineConfigDialog() {
	QPointer<QDialog> dlg = new QDialog(this);
	dlg->setWindowTitle(i18n("Chess Engines"));

	auto bBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
	connect( bBox, &QDialogButtonBox::accepted, dlg, &QDialog::accept );
	connect( bBox, &QDialogButtonBox::rejected, dlg, &QDialog::reject );

	QVBoxLayout *layout = new QVBoxLayout;
	EngineSettings* ecd = new EngineSettings(dlg);
	layout->addWidget(ecd);
	layout->addWidget(bBox);
	dlg->setLayout(layout);
	dlg->setFocus();

	if (dlg->exec() == QDialog::Accepted) {
		ecd->save();
		loadEngines();
	}
	delete dlg;
}

void GameDialog::loadEngines() {
	QStringList programs;
	QList<EngineConfiguration> configs;
	for (const QString& s : Settings::engineConfigurations()) {
		const  QStringList l = s.split(QLatin1Char(':'), QString::SkipEmptyParts);
		EngineConfiguration e = EngineConfiguration(s);
		if(!e.name.isEmpty()) {
			programs << e.name;
			configs << e;
		}
	}

	ui->cbPlayer1Engine->clear();
	ui->cbPlayer1Engine->addItems(programs);
	if(!Settings::player1Program().isEmpty())
		ui->cbPlayer1Engine->setCurrentItem(Settings::player1Program(), false);
	else
		ui->cbPlayer1Engine->setCurrentIndex(0);

	ui->cbPlayer2Engine->clear();
	ui->cbPlayer2Engine->addItems(programs);
	if(!Settings::player2Program().isEmpty())
		ui->cbPlayer2Engine->setCurrentItem(Settings::player2Program(), false);
	else
		ui->cbPlayer2Engine->setCurrentIndex(0);

	checkOkButton();
}

void GameDialog::checkOkButton() {
	if (ui->rbPlayer1Engine->isChecked() && ui->cbPlayer1Engine->currentIndex() == -1) {
		okButton->setEnabled(false);
		okButton->setToolTip(i18n("Select a chess engine for the first player."));
		return;
	}

	if (ui->rbPlayer2Engine->isChecked() && ui->cbPlayer2Engine->currentIndex() == -1) {
		okButton->setEnabled(false);
		okButton->setToolTip(i18n("Select a chess engine for the second player."));
		return;
	}

	okButton->setEnabled(true);
	okButton->setToolTip(i18n("Start the game."));
}
