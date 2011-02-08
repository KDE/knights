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

#include "clockwidget.h"

#include "ui_clockwidget.h"

#include <KDebug>
#include <QtCore/QTimer>
#include <QtCore/QTime>

using namespace Knights;

ClockWidget::ClockWidget ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::ClockWidget;
    ui->setupUi ( this );
}

ClockWidget::~ClockWidget()
{
    delete ui;
}

void ClockWidget::setDisplayedPlayer ( Color color )
{
    bool w = ( color == White );
    ui->verticalLayout->addWidget ( w ? ui->groupB : ui->groupW );
    ui->verticalLayout->addWidget ( w ? ui->groupW : ui->groupB );
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
    const int units = miliSeconds / 100;
    QProgressBar* bar = ( color == White ) ? ui->progressW : ui->progressB;
    if ( units > bar->maximum() )
    {
        bar->setMaximum ( units );
        updateTimeFormat();
    }
    bar->setValue ( units );
    bar->setFormat ( time.toString( m_timeFormat ) );

    Clock* clock = ( color == White ) ? ui->clockW : ui->clockB;
    clock->setTime ( time );
}

void ClockWidget::setTimeLimit ( Color color, const QTime& time )
{
    kDebug() << color << time;
    m_timeLimit[color] = time;
    int seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
    switch ( color )
    {
        case White:
            ui->progressW->setMaximum ( seconds * 10 );
            break;
        case Black:
            ui->progressB->setMaximum ( seconds * 10 );
            break;
        default:
            break;
    }
    updateTimeFormat();
    setCurrentTime( color, time );
}

void ClockWidget::updateTimeFormat()
{
    if ( m_timeLimit[White] > QTime(1,0) || m_timeLimit[Black] > QTime(1,0) )
    {
        m_timeFormat = QLatin1String("h:mm:ss");
    }
    else
    {
        m_timeFormat = QLatin1String("mm:ss");
    }
}


// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
