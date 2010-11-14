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

#include "clockwidget.h"

#include "ui_clockwidget.h"

#include <KDebug>
#include <QtCore/QTimer>
#include <QtCore/QTime>

using namespace Knights;

const int timerInterval = 100; // update the time every timerInterval miliseconds

ClockWidget::ClockWidget ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::ClockWidget;
    ui->setupUi ( this );
    m_box[White] = ui->groupW;
    m_box[Black] = ui->groupB;

    m_timeIncrement[White] = 0;
    m_timeIncrement[Black] = 0;
}

ClockWidget::~ClockWidget()
{
    delete ui;
}

void ClockWidget::setActivePlayer ( Color color )
{
    killTimer ( m_timerId[m_activePlayer] );
    if ( !m_started [ color ] )
    {
        m_started [ color ] = true;
        return;
    }
    incrementTime ( m_activePlayer, m_timeIncrement[m_activePlayer] );
    m_timerId[color] = startTimer ( timerInterval );
    m_activePlayer = color;
}

void ClockWidget::setDisplayedPlayer ( Color color )
{
    ui->verticalLayout->addWidget ( m_box[oppositeColor ( color ) ] );
    ui->verticalLayout->addWidget ( m_box[color] );
}

void ClockWidget::setPlayerName ( Color color, const QString& name )
{
    switch ( color )
    {
        case White:
            ui->groupW->setTitle ( name );
            break;
        case Black:
            ui->groupB->setTitle ( name );
            break;
        default:
            break;
    }
}

void ClockWidget::setCurrentTime ( Color color, const QTime& time )
{
    m_currentTime[color] = time;
    
    const int miliSeconds = time.hour() * 3600 * 1000 + time.minute() * 60 * 1000 + time.second() * 1000 + time.msec();
    const int units = miliSeconds / timerInterval;
    QProgressBar* bar = ( color == White ) ? ui->progressW : ui->progressB;
    if ( units > bar->maximum() )
    {
        bar->setMaximum ( units );
    }
    bar->setValue ( units );
    bar->setFormat ( time.toString( QLatin1String("h:mm:ss") ) );

    Clock* clock = ( color == White ) ? ui->clockW : ui->clockB;
    clock->setTime ( time );
}

void ClockWidget::setTimeLimit ( Color color, const QTime& time )
{
    m_timeLimit[color] = time;
    int seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
    switch ( color )
    {
        case White:
            ui->progressW->setMaximum ( seconds * 1000 / timerInterval );
            break;
        case Black:
            ui->progressB->setMaximum ( seconds * 1000 / timerInterval );
            break;
        default:
            break;
    }
    setCurrentTime( color, time );
}

void ClockWidget::setTimeIncrement ( Color color, int seconds )
{
    m_timeIncrement[color] = 1000 * seconds;
}

void ClockWidget::incrementTime ( Color color, int miliseconds )
{
    switch ( color )
    {
        case White:
            setCurrentTime ( White, m_currentTime[White].addMSecs ( miliseconds ) );
            if ( ui->progressW->value() <= 0 )
            {
                emit timeOut ( White );
                emit opponentTimeOut ( Black );
            }
            break;
        case Black:
            setCurrentTime ( Black, m_currentTime[Black].addMSecs ( miliseconds ) );
            if ( ui->progressB->value() <= 0 )
            {
                emit timeOut ( Black );
                emit opponentTimeOut ( White );
            }
            break;
        default:
            break;
    }
}

void Knights::ClockWidget::timerEvent ( QTimerEvent* event )
{
    Q_UNUSED ( event )
    incrementTime ( m_activePlayer, -timerInterval );
}

void ClockWidget::pauseClock()
{
    killTimer ( m_timerId[m_activePlayer] );
}

void ClockWidget::resumeClock()
{
    m_timerId[m_activePlayer] = startTimer ( timerInterval );
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
