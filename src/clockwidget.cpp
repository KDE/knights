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
#include <QTimer>

#include <KDebug>


using namespace Knights;

ClockWidget::ClockWidget ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
  ui = new Ui::ClockWidget;
  ui->setupUi(this);
  kDebug() << ui->progressB->invertedAppearance() << ui->progressW->invertedAppearance();
  m_box[Piece::White] = ui->groupW;
  m_box[Piece::Black] = ui->groupB;
}

ClockWidget::~ClockWidget()
{
  delete ui;
}


void ClockWidget::setActivePlayer ( Piece::Color color )
{
  killTimer(m_timerId[Piece::oppositeColor(color)]);
  m_timerId[color] = startTimer(100);
  m_activePlayer = color;
}


void ClockWidget::setDisplayedPlayer ( Piece::Color color )
{
  ui->verticalLayout->addWidget(m_box[Piece::oppositeColor(color)]);
  ui->verticalLayout->addWidget(m_box[color]);
}

void ClockWidget::setPlayerName ( Piece::Color color, QString name )
{
  switch (color)
  {
    case Piece::White:
      ui->groupW->setTitle(name);
      break;
    case Piece::Black:
      ui->groupB->setTitle(name);
      break;
    default:
      break;
  }
}

void ClockWidget::setTimeLimit ( Piece::Color color, QTime time )
{
  m_timeLimit[color] = time;
  int seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
  switch(color)
  {
    case Piece::White:
      ui->timeW->setTime(time);
      ui->progressW->setMaximum(10 * seconds);
      ui->progressW->setValue(10 * seconds);
      kDebug() << ui->progressW->maximum();
      break;
    case Piece::Black:
      ui->timeB->setTime(time);
      ui->progressB->setMaximum(10 * seconds);
      ui->progressB->setValue(10 * seconds);
      kDebug() << ui->progressB->maximum();
      break;
    default:
      break;
  }
}

void Knights::ClockWidget::timerEvent ( QTimerEvent* event )
{
  Q_UNUSED(event)
  switch (m_activePlayer)
  {
    case Piece::White:
      ui->progressW->setValue(ui->progressW->value() - 1);
      ui->timeW->setTime(ui->timeW->time().addMSecs(-100));
      if (ui->progressW->value() <= 0)
      {
        emit timeOut(Piece::White);
        emit opponentTimeOut(Piece::Black);
      }
      break;
    case Piece::Black:
      ui->progressB->setValue(ui->progressB->value() - 1);
      ui->timeB->setTime(ui->timeB->time().addMSecs(-100));
      if (ui->progressB->value() <= 0)
      {
        emit timeOut(Piece::Black);
        emit opponentTimeOut(Piece::White);
      }
      break;
    default:
      break;
  }
}

void ClockWidget::pauseClock()
{
  killTimer(m_timerId[m_activePlayer]);
}

void ClockWidget::resumeClock()
{
  m_timerId[m_activePlayer] = startTimer(100);
}

