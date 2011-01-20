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
#include "proto/localprotocol.h"
#include "proto/xboardproto.h"
#include "proto/ficsprotocol.h"
#include "gamemanager.h"

using namespace Knights;

GameDialog::GameDialog ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::GameDialog();
    ui->setupUi ( this );
    setObjectName ( QLatin1String ( "GameDialogWidget" ) );

    ui->timeGroupBasic->setEnabled ( Settings::timeEnabled() );

    ui->startingTime->setTime ( Settings::playerTime().time() );
    ui->timeIncrement->setTime ( Settings::playerTimeIncrement().time() );
    ui->numberOfMoves->setValue ( Settings::playerMoves() );

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

    ui->whiteProgram->setHistoryItems( Settings::programs() );
    ui->whiteProgram->setCurrentItem( Settings::currentProgram(), true );
    ui->whiteServer->setHistoryItems( Settings::servers() );
    ui->whiteServer->setCurrentItem( Settings::currentServer(), true );
    
    ui->blackProgram->setHistoryItems( Settings::programs() );
    ui->blackProgram->setCurrentItem( Settings::currentProgram(), true );
    ui->blackServer->setHistoryItems( Settings::servers() );
    ui->blackServer->setCurrentItem( Settings::currentServer(), true );
}

GameDialog::~GameDialog()
{
    delete ui;
}

GameDialog::FicsMode GameDialog::ficsMode()
{
    if ( ui->tabWidget->currentIndex() == 0 )
    {
        return ui->oppFics->isChecked() ? PlayerVsFics : NoFics;
    }
    else
    {
        if ( ui->whiteFics->isChecked() && ui->blackFics->isChecked() )
        {
            return BothPlayersFics;
        }
        else if ( ui->whiteFics->isChecked() || ui->blackFics->isChecked() )
        {
            return PlayerVsFics;
        }
        else
        {
            return NoFics;
        }
    }
}

Color GameDialog::ficsColor()
{
    if ( ui->tabWidget->currentIndex() == 0 )
    {
        if ( ui->colorRandom->isChecked() )
        {
            return NoColor;
        }
        else if ( ui->colorWhite->isChecked() )
        {
            return White;
        }
        else
        {
            return Black;
        }
    }
    else
    {
        if ( ui->whiteFics->isChecked() && ui->blackFics->isChecked() )
        {
            return NoColor;
        }
        else if ( ui->whiteFics->isChecked() )
        {
            return White;
        }
        else if ( ui->blackFics->isChecked() )
        {
            return Black;
        }
        else
        {
            return NoColor;
        }
    }
}

void GameDialog::setupProtocols()
{
    if ( ui->tabWidget->currentIndex() == 0 )
    {
        // Basic / normal settings
        Protocol* player = new LocalProtocol();
        Protocol* opp;
        if ( ui->oppComp->isChecked() )
        {
            opp = new XBoardProtocol();
            opp->setAttribute("program", ui->programComboBox->currentText());
        }
        else if ( ui->oppHuman->isChecked() )
        {
            opp = new LocalProtocol();
        }
        else
        {
            opp = new FicsProtocol();
            opp->setAttribute("server", ui->serverComboBox->currentText());
            opp->setAttribute("port", 5000);
        }
        Color color = NoColor;
        if ( ui->colorBlack->isChecked() )
        {
            color = Black;
        }
        else if ( ui->colorWhite->isChecked() )
        {
            color = White;
        }
        else
        {
            color = ( qrand() % 2 ) ? White : Black;
        }
        if ( color == White )
        {
            Protocol::setWhiteProtocol(player);
            Protocol::setBlackProtocol(opp);
        }
        else
        {
            Protocol::setWhiteProtocol(opp);
            Protocol::setBlackProtocol(player);
        }
    }
    else
    {
        if ( ui->whiteHuman->isChecked() )
        {
            Protocol::setWhiteProtocol(new LocalProtocol);
        }
        else if ( ui->whiteComp->isChecked() )
        {
            Protocol::setWhiteProtocol(new XBoardProtocol);
            Protocol::white()->setAttribute("program", ui->whiteProgram->currentText());
        }
        else
        {
            Protocol::setWhiteProtocol(new FicsProtocol);
            Protocol::white()->setAttribute("server", ui->whiteServer->currentText());
            Protocol::white()->setAttribute("port", 5000);
        }
        if ( ui->blackHuman->isChecked() )
        {
            Protocol::setBlackProtocol(new LocalProtocol);
        }
        else if ( ui->blackComp->isChecked() )
        {
            Protocol::setBlackProtocol(new XBoardProtocol);
            Protocol::black()->setAttribute("program", ui->blackProgram->currentText());
        }
        else
        {
            Protocol::setBlackProtocol(new FicsProtocol);
            Protocol::black()->setAttribute("server", ui->blackServer->currentText());
            Protocol::black()->setAttribute("port", 5000);
        }
    }
    TimeControl tc;
    tc.baseTime = ui->startingTime->time();
    tc.moves = ui->numberOfMoves->value();
    tc.increment = QTime().secsTo ( ui->timeIncrement->time() );
    Protocol::white()->setTimeControl ( tc );
    Protocol::black()->setTimeControl ( tc );
}


void GameDialog::writeConfig()
{
    Settings::EnumProtocol::type selectedProtocol = Settings::EnumProtocol::None;
    if ( ui->oppComp->isChecked() )
    {
        selectedProtocol = Settings::EnumProtocol::XBoard;
        QStringList programs;
        programs << ui->programComboBox->historyItems();
        programs << ui->whiteProgram->historyItems();
        programs << ui->blackProgram->historyItems();
        programs.removeDuplicates();
        Settings::setPrograms( programs );
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

    bool timeLimitEnabled = ui->timeGroupBasic->isChecked();
    Settings::setProtocol ( selectedProtocol );
    Settings::setColor ( selectedColor );
    Settings::setTimeEnabled ( timeLimitEnabled );
    if ( timeLimitEnabled )
    {
        Settings::setPlayerTime ( QDateTime ( QDate::currentDate(), ui->startingTime->time() ) );
        Settings::setPlayerTimeIncrement( QDateTime ( QDate::currentDate(), ui->timeIncrement->time() ) );
    }
    Settings::self()->writeConfig();
}

void GameDialog::updateTimeEdits()
{
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
