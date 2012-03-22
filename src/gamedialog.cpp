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

#include "gamedialog.h"

#include "ui_gamedialog.h"
#include "proto/localprotocol.h"
#include "proto/xboardprotocol.h"
#include "proto/ficsprotocol.h"
#include "gamemanager.h"

#include <Solid/Networking>
#include "enginesettings.h"
#include "proto/uciprotocol.h"

using namespace Knights;

GameDialog::GameDialog ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::GameDialog();
    ui->setupUi ( this );
    setObjectName ( QLatin1String ( "GameDialogWidget" ) );

    ui->timeGroup->setChecked ( Settings::timeEnabled() );

    ui->timeLimit->setValue ( Settings::timeLimit() );
    ui->timeIncrement->setValue ( Settings::timeIncrement() );
    ui->numberOfMoves->setValue ( Settings::numberOfMoves() );

    switch ( Settings::player1Protocol() )
    {
        case Settings::EnumPlayer1Protocol::Local:
            ui->player1Human->setChecked ( true );
            break;
        case Settings::EnumPlayer1Protocol::XBoard:
            ui->player1Comp->setChecked ( true );
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
    
    switch ( Settings::player2Protocol() )
    {
        case Settings::EnumPlayer2Protocol::Local:
            ui->player2Human->setChecked ( true );
            break;
        case Settings::EnumPlayer2Protocol::XBoard:
            ui->player2Comp->setChecked ( true );
            break;
        case Settings::EnumPlayer2Protocol::Fics:
            ui->player2Fics->setChecked ( true );
            break;
    }

    loadEngines();

    ui->player2Server->setHistoryItems ( Settings::servers() );
    ui->player2Server->setEditText ( Settings::currentServer() ); 

    connect ( ui->player1Engines, SIGNAL(clicked(bool)), SLOT(showEngineConfigDialog()) );
    connect ( ui->player2Engines, SIGNAL(clicked(bool)), SLOT(showEngineConfigDialog()) );
    
    connect ( ui->timeLimit, SIGNAL(valueChanged(int)), SLOT(updateTimeEdits()) );
    connect ( ui->timeIncrement, SIGNAL(valueChanged(int)), SLOT(updateTimeEdits()) );
    connect ( ui->numberOfMoves, SIGNAL(valueChanged(int)), SLOT(updateTimeEdits()) );
    connect ( Solid::Networking::notifier(), SIGNAL(statusChanged(Solid::Networking::Status)), SLOT(changeNetworkStatus(Solid::Networking::Status)) );
    
    changeNetworkStatus(Solid::Networking::status());
    updateTimeEdits();
}

GameDialog::~GameDialog()
{
    delete ui;
}

void GameDialog::setupProtocols()
{
    TimeControl tc;
    tc.baseTime = ui->timeGroup->isChecked() ? QTime().addSecs( 60 * ui->timeLimit->value() ) : QTime();
    tc.moves = ui->player2Fics->isChecked() ? 0 : ui->numberOfMoves->value();
    tc.increment = ui->timeIncrement->value();
    Manager::self()->setTimeControl(NoColor, tc);
    
    QList<EngineConfiguration> configs;
    foreach ( const QString& s, Settings::engineConfigurations() )
    {
        configs << EngineConfiguration ( s );
    }

    Protocol* p1 = 0;
    Protocol* p2 = 0;
    Color c1 = NoColor;
    if ( ui->player1Human->isChecked() )
    {
        p1 = new LocalProtocol;
    }
    else
    {
        if ( ui->player2Program->currentIndex() != -1 )
        {
            EngineConfiguration c = configs [ ui->player1Program->currentIndex() ];
            if ( c.iface == EngineConfiguration::XBoard )
            {
                p1 = new XBoardProtocol;
            }
            else
            {
                p1 = new UciProtocol; 
            }
            p1->setAttribute ( "program", c.commandLine );
            p1->setPlayerName ( c.name );
        }
        else
        {
            //This happens when the user didn't select any game engine, so we are
            //creating below a dummy XBoardProtocol object with an invalid command
            //path (i.e. an empty QString), thus this object will send a signal
            //error which will end up in a notificaction of the error to the user,
            //raising a KMessageBox error or so.
            p1 = new XBoardProtocol;
            p1->setAttribute( "program", QString());
        }  
    }
    
    if ( ui->colorWhite->isChecked() )
    {
        c1 = White;
    }
    else if ( ui->colorBlack->isChecked() )
    {
        c1 = Black;
    }
    
    if ( ui->player2Human->isChecked() )
    {
        p2 = new LocalProtocol;
    }
    else if ( ui->player2Comp->isChecked() )
    {
        if ( ui->player2Program->currentIndex() != -1 )
        {
            EngineConfiguration c = configs [ ui->player2Program->currentIndex() ];
            if ( c.iface == EngineConfiguration::XBoard )
            {
                p2 = new XBoardProtocol;
            }
            else
            {
                p2 = new UciProtocol; 
            }
            p2->setAttribute ( "program", c.commandLine );
            p2->setPlayerName ( c.name );
        }
        else
        {
            //This happens when the user didn't select any game engine, so we are
            //creating below a dummy XBoardProtocol object with an invalid command
            //path (i.e. an empty QString), thus this object will send a signal
            //error which will end up in a notificaction of the error to the user,
            //raising a KMessageBox error or so.
            p2 = new XBoardProtocol;
            p2->setAttribute( "program", QString());
        }
    }
    else
    {
        p2 = new FicsProtocol;
        p2->setAttribute ( "server", ui->player2Server->currentText() );
    }
    if ( c1 == NoColor )
    {
        qsrand(QTime::currentTime().msec());
        c1 = ( qrand() % 2 ) ? White : Black;
    }
    // Color-changing by the FICS protocol happens later, so it doesn't matter what we do here. 
    Protocol::setWhiteProtocol ( c1 == White ? p1 : p2 );
    Protocol::setBlackProtocol ( c1 == White ? p2 : p1 );
}


void GameDialog::writeConfig()
{
    Settings::EnumPlayer1Protocol::type p1;
    if ( ui->player1Human->isChecked() )
    {
        p1 = Settings::EnumPlayer1Protocol::Local;
    }
    else
    {
        p1 = Settings::EnumPlayer1Protocol::XBoard;
        Settings::setPlayer1Program ( ui->player1Program->currentText() );
    }
    Settings::setPlayer1Protocol ( p1 );
    
    Settings::EnumPlayer2Protocol::type p2;
    if ( ui->player2Human->isChecked() )
    {
        p2 = Settings::EnumPlayer2Protocol::Local;
    }
    else if ( ui->player2Comp->isChecked() )
    {
        p2 = Settings::EnumPlayer2Protocol::XBoard;
        Settings::setPlayer2Program ( ui->player2Program->currentText() );
    }
    else
    {
        p2 = Settings::EnumPlayer2Protocol::Fics;
        Settings::setServers ( ui->player2Server->historyItems() );
        Settings::setCurrentServer ( ui->player2Server->currentText() );
    }   
    Settings::setPlayer2Protocol ( p2 );
    
    Settings::EnumColor::type selectedColor = Settings::EnumColor::NoColor;
    if ( ui->colorWhite->isChecked() )
    {
        selectedColor = Settings::EnumColor::White;
    }
    else if ( ui->colorBlack->isChecked() )
    {
        selectedColor = Settings::EnumColor::Black;
    }
    Settings::setColor ( selectedColor );
    
    bool timeLimitEnabled = ui->timeGroup->isChecked();
    Settings::setTimeEnabled ( timeLimitEnabled );
    if ( timeLimitEnabled )
    {
        Settings::setTimeLimit ( ui->timeLimit->value() );
        Settings::setTimeIncrement( ui->timeIncrement->value() );
        if ( p2 != Settings::EnumPlayer2Protocol::Fics )
        {
            Settings::setNumberOfMoves ( ui->numberOfMoves->value() );
        }
    }
    
    Settings::self()->writeConfig();
}

void GameDialog::updateTimeEdits()
{
    ui->timeLimit->setSuffix ( i18np ( " minute", " minutes", ui->timeLimit->value() ) );
    ui->timeIncrement->setSuffix ( i18np ( " second", " seconds", ui->timeIncrement->value() ) );
    ui->numberOfMoves->setSuffix ( i18np ( " move", " moves", ui->numberOfMoves->value() ) );
}

void GameDialog::changeNetworkStatus(Solid::Networking::Status status)
{
    kDebug() << status;
    bool enableFics = ( status == Solid::Networking::Connected || status == Solid::Networking::Unknown );
    if (!enableFics && ui->player2Fics->isChecked())
    {
        ui->player2Comp->setChecked ( true );
    }
    ui->player2Fics->setEnabled ( enableFics );
}

void GameDialog::showEngineConfigDialog()
{
    KDialog* dlg = new KDialog ( this );
    EngineSettings* ecd = new EngineSettings ( dlg );
    dlg->setMainWidget ( ecd );
    connect ( dlg, SIGNAL(accepted()), ecd, SLOT(writeConfig()) );
    connect ( dlg, SIGNAL(accepted()), this, SLOT(loadEngines()) );
    dlg->show();
}

void GameDialog::loadEngines()
{
    QStringList programs;
    QList<EngineConfiguration> configs;
    foreach ( const QString& s, Settings::engineConfigurations() )
    {
        QStringList l = s.split ( QLatin1Char(':'), QString::SkipEmptyParts );
        EngineConfiguration e = EngineConfiguration ( s );
        if ( !e.name.isEmpty() )
        {
            programs << e.name;
            configs << e;
        }
    }
       
    ui->player1Program->clear();
    ui->player2Program->clear();
    
    ui->player1Program->addItems ( programs );
    ui->player1Program->setCurrentItem ( Settings::player1Program(), false );
    
    ui->player2Program->addItems ( programs );
    ui->player2Program->setCurrentItem ( Settings::player2Program(), false );
}




// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
