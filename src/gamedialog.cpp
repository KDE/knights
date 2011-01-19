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

using namespace Knights;

GameDialog::GameDialog ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::GameDialog();
    ui->setupUi ( this );
    setObjectName ( QLatin1String ( "GameDialogWidget" ) );
    connect ( ui->timeGroup, SIGNAL ( toggled ( bool ) ), this, SLOT ( timeEnabled ( bool ) ) );
    connect ( ui->oppHuman, SIGNAL ( toggled ( bool ) ), this, SLOT ( hotseatModeToggled ( bool ) ) );
    connect ( ui->oppFics, SIGNAL ( toggled ( bool ) ), this, SLOT ( ficsModeToggled ( bool ) ) );

    m_timeEnabled = Settings::timeEnabled();
    ui->timeGroup->setChecked ( m_timeEnabled );

    ui->startingTimeBasic->setTime ( Settings::playerTime().time() );
    ui->timeIncrementBasic->setTime ( Settings::playerTimeIncrement().time() );
    ui->numberOfMovesBasic->setValue ( Settings::playerMoves() );

    switch ( Settings::protocol() )
    {
        case Settings::EnumProtocol::XBoard:
            ui->oppComp->setChecked ( true );
            break;
        case Settings::EnumProtocol::FICS:
            ui->oppFics->setChecked ( true );
            break;
        default:
            ui->oppHuman->setChecked ( true );
            break;
    }

    switch ( Settings::color() )
    {
        case Settings::EnumColor::NoColor:
            ui->colorRandom->setChecked ( true );
            break;
        case Settings::EnumColor::White:
            ui->colorWhite->setChecked ( true );
            break;
        case Settings::EnumColor::Black:
            ui->colorBlack->setChecked ( true );
            break;
    }

    ui->programComboBox->setHistoryItems( Settings::programs() );
    ui->programComboBox->setCurrentItem( Settings::currentProgram(), true );
    ui->serverComboBox->setHistoryItems( Settings::servers() );
    ui->serverComboBox->setCurrentItem( Settings::currentServer(), true );

    hotseatModeToggled ( ui->oppHuman->isChecked() );
    ficsModeToggled ( ui->oppFics->isChecked() );
}

GameDialog::~GameDialog()
{
    delete ui;
}

void GameDialog::writeConfig()
{
    Settings::EnumProtocol::type selectedProtocol = Settings::EnumProtocol::None;
    if ( ui->oppComp->isChecked() )
    {
        selectedProtocol = Settings::EnumProtocol::XBoard;
        Settings::setPrograms( ui->programComboBox->historyItems() );
        Settings::setCurrentProgram( ui->programComboBox->currentText() );
    }
    else if ( ui->oppFics->isChecked() )
    {
        selectedProtocol = Settings::EnumProtocol::FICS;
        Settings::setServers( ui->serverComboBox->historyItems() );
        Settings::setCurrentServer( ui->serverComboBox->currentText() );
    }

    Settings::EnumColor::type selectedColor = Settings::EnumColor::NoColor;
    if ( ui->colorWhite->isChecked() )
    {
        selectedColor = Settings::EnumColor::White;
    }
    else if ( ui->colorBlack->isChecked() )
    {
        selectedColor = Settings::EnumColor::Black;
    }

    bool timeLimitEnabled = ui->timeGroup->isChecked();
    Settings::setProtocol ( selectedProtocol );
    Settings::setColor ( selectedColor );
    Settings::setTimeEnabled ( timeLimitEnabled );
    if ( timeLimitEnabled )
    {
        Settings::setPlayerTime ( QDateTime ( QDate::currentDate(), ui->startingTimeBasic->time() ) );
        Settings::setPlayerTimeIncrement( QDateTime ( QDate::currentDate(), ui->timeIncrementBasic->time() ) );
    }
    Settings::self()->writeConfig();
}

Color GameDialog::color() const
{
    if ( ui->colorBlack->isChecked() )
    {
        return Black;
    }
    else if ( ui->colorWhite->isChecked() )
    {
        return White;
    }
    else
    {
        return NoColor;
    }
}

bool GameDialog::timeLimit() const
{
    return ui->timeGroup->isChecked();
}

QTime GameDialog::opponentTime() const
{
    return ui->oppTimeEdit->time();
}

int GameDialog::opponentIncrement() const
{
    return ui->incTimeRadio->isChecked() ? QTime().secsTo ( ui->oppIncTimeEdit->time() ) : 0;
}

QTime GameDialog::playerTime() const
{
    return ui->playerTimeEdit->time();
}

int GameDialog::playerIncrement() const
{
    return ui->incTimeRadio->isChecked() ? QTime().secsTo ( ui->oppIncTimeEdit->time() ) : 0;
}

Settings::EnumProtocol::type GameDialog::protocol() const
{
    if ( ui->oppComp->isChecked() )
    {
        return Settings::EnumProtocol::XBoard;
    }
    else if ( ui->oppFics->isChecked() )
    {
        return Settings::EnumProtocol::FICS;
    }
    else
    {
        return Settings::EnumProtocol::None;
    }
}

QString GameDialog::program() const
{
    if ( ui->oppComp->isChecked() )
    {
        return ui->programComboBox->currentText();
    }
    else
    {
        return QString();
    }
}

QString GameDialog::server() const
{
    if ( ui->oppFics->isChecked() )
    {
        return ui->serverComboBox->currentText();
    }
    else
    {
        return QString();
    }
}


void GameDialog::sameTimeChanged ( bool enabled )
{
    if ( !m_forceSameTime )
    {
        m_sameTime = enabled;
        updateTimeEdits();
    }
}


void GameDialog::hotseatModeToggled ( bool enabled )
{
    if ( enabled )
    {
        ui->playerLabel->setText ( i18n ( "White" ) );
        ui->oppLabel->setText ( i18n ( "Black" ) );
        ui->sameTimeCheckBox->setEnabled ( true );
    }
    else
    {
        ui->playerLabel->setText ( i18n ( "Player" ) );
        ui->oppLabel->setText ( i18n ( "Opponent" ) );

        ui->sameTimeCheckBox->setChecked ( true );
        ui->sameTimeCheckBox->setEnabled ( false );
    }
    updateTimeEdits();
}

void GameDialog::timeEnabled ( bool enabled )
{
    m_timeEnabled = enabled;
    updateTimeEdits();
}

void GameDialog::updateTimeEdits()
{
    if ( !m_timeEnabled )
    {
        //Nothin to do, since the group box is already disabled
        return;
    }
    if ( ui->sameTimeCheckBox->isChecked() )
    {
        connect ( ui->playerTimeEdit, SIGNAL ( timeChanged ( QTime ) ), ui->oppTimeEdit, SLOT ( setTime ( QTime ) ) );
        connect ( ui->playerIncTimeEdit, SIGNAL ( timeChanged ( QTime ) ), ui->oppIncTimeEdit, SLOT ( setTime ( QTime ) ) );
        connect ( ui->playerMoves, SIGNAL(valueChanged(int)), ui->oppMoves, SLOT(setValue(int)) );
        ui->oppTimeEdit->setTime ( ui->playerTimeEdit->time() );
        ui->oppIncTimeEdit->setTime ( ui->playerIncTimeEdit->time() );
        ui->oppMoves->setValue ( ui->playerMoves->value() );
    }
    else
    {
        disconnect ( ui->playerTimeEdit, SIGNAL ( timeChanged ( QTime ) ), ui->oppTimeEdit, SLOT ( setTime ( QTime ) ) );
        disconnect ( ui->playerIncTimeEdit, SIGNAL ( timeChanged ( QTime ) ), ui->oppIncTimeEdit, SLOT ( setTime ( QTime ) ) );
        disconnect ( ui->playerMoves, SIGNAL(valueChanged(int)), ui->oppMoves, SLOT(setValue(int)) );
    }
}

int GameDialog::playerMoves()
{
    return ui->conventionalTimeRadio->isChecked() ? ui->playerMoves->value() : 0;
}

int GameDialog::opponentMoves()
{
    return ui->conventionalTimeRadio->isChecked() ? ui->oppMoves->value() : 0;
}

void GameDialog::ficsModeToggled ( bool enabled )
{
    if ( enabled )
    {
        ui->colorRandom->setText ( i18n ( "Choose &later" ) );
    }
    else
    {
        ui->colorRandom->setText ( i18n ( "&Random" ) );
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
