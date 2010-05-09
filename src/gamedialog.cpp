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

#include "gamedialog.h"

#include "ui_gamedialog.h"
#include <QTimer>


using namespace Knights;


GameDialog::GameDialog(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  ui = new Ui::GameDialog();
    ui->setupUi(this);
    setObjectName("GameDialogWidget");
    connect(ui->timeCheckBox, SIGNAL(toggled(bool)), this, SLOT(timeEnabled(bool)));
    connect(ui->sameTimeCheckBox, SIGNAL(toggled(bool)), this, SLOT(sameTimeChanged(bool)));
    connect(ui->oppHuman, SIGNAL(toggled(bool)), this, SLOT(hotseatModeToggled(bool)));

    m_timeEnabled = Settings::timeEnabled();
    ui->timeCheckBox->setChecked(m_timeEnabled);

    m_sameTime  = Settings::sameTime();
    ui->sameTimeCheckBox->setChecked(m_sameTime);
    
    ui->playerTimeEdit->setTime(Settings::playerTime().time());
    ui->oppTimeEdit->setTime(Settings::opponentTime().time());

    updateTimeEdits();

    switch (Settings::protocol())
    {
      case Settings::EnumProtocol::None:
        ui->oppHuman->setChecked(true);
        break;
      case Settings::EnumProtocol::XBoard:
        ui->oppComp->setChecked(true);
        break;
      default:
        break;
    }
}


GameDialog::~GameDialog()
{
  Settings::EnumProtocol::type selectedProtocol = Settings::EnumProtocol::None;
  if (ui->oppComp->isChecked())
  {
    selectedProtocol = Settings::EnumProtocol::XBoard;
  }
  bool timeLimitEnabled = ui->timeCheckBox->isChecked();
  Settings::setProtocol(selectedProtocol);
  Settings::setTimeEnabled(timeLimitEnabled);
  Settings::setSameTime(ui->sameTimeCheckBox->isChecked());
  Settings::setPlayerTime(QDateTime(QDate::currentDate(), ui->playerTimeEdit->time()));
  Settings::setOpponentTime(QDateTime(QDate::currentDate(), ui->oppTimeEdit->time()));
  Settings::self()->writeConfig();
  delete ui;
}


Piece::Color GameDialog::color() const
{
    if (ui->colorBlack->isChecked())
    {
        return Piece::Black;
    }
    else if (ui->colorWhite->isChecked())
    {
        return Piece::White;
    }
    else
    {
        return Piece::NoColor;
    }
}

bool GameDialog::timeLimit()
{
  return ui->timeCheckBox->isChecked();
}

QTime GameDialog::opponentTime() const
{
    return ui->oppTimeEdit->time();
}

QTime GameDialog::playerTime() const
{
    return ui->playerTimeEdit->time();
}

Settings::EnumProtocol::type GameDialog::protocol() const
{
    if (ui->oppComp->isChecked())
    {
        return Settings::EnumProtocol::XBoard;
    }
    else
    {
        return Settings::EnumProtocol::None;
    }
}

QString GameDialog::program()
{
    if (ui->oppComp->isChecked())
    {
        return ui->khistorycombobox->currentText();
    }
    else
    {
        return QString();
    }
}


void GameDialog::sameTimeChanged ( bool enabled )
{
  m_sameTime = enabled;
  updateTimeEdits();
}


void GameDialog::hotseatModeToggled ( bool enabled )
{
  if (enabled)
  {
    ui->playerLabel->setText(i18n("White"));
    ui->oppLabel->setText(i18n("Black"));
  }
  else
  {
    ui->playerLabel->setText(i18n("Player"));
    ui->oppLabel->setText(i18n("Opponent"));
  }
}

void GameDialog::timeEnabled ( bool enabled )
{
  m_timeEnabled = enabled;
  updateTimeEdits();
}

void GameDialog::updateTimeEdits()
{
  if (!m_timeEnabled)
  {
    ui->sameTimeCheckBox->setEnabled(false);
    ui->playerTimeEdit->setEnabled(false);
    ui->oppTimeEdit->setEnabled(false);
  }
  else if (m_sameTime)
  {
    ui->sameTimeCheckBox->setEnabled(true);
    ui->playerTimeEdit->setEnabled(true);
    ui->oppTimeEdit->setEnabled(false);
    connect(ui->playerTimeEdit, SIGNAL(timeChanged(QTime)), ui->oppTimeEdit, SLOT(setTime(QTime)));
  }
  else
  {
    ui->sameTimeCheckBox->setEnabled(true);
    ui->playerTimeEdit->setEnabled(true);
    ui->oppTimeEdit->setEnabled(true);
    disconnect(ui->playerTimeEdit, SIGNAL(timeChanged(QTime)), ui->oppTimeEdit, SLOT(setTime(QTime)));
  }
}




