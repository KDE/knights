/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "gamemanager.h"

#include "core/move.h"

#include <QStack>
#include "proto/protocol.h"
#include <KDebug>

using namespace Knights;

const int TimerInterval = 100;

class Knights::GameManagerPrivate
{
public:
  Color activePlayer;
  bool running;
  int timer;

  TimeControl whiteTimeControl;
  TimeControl blackTimeControl;

  QStack<Move> moveHistory;
  QStack<Move> moveUndoStack;
};

K_GLOBAL_STATIC(Manager, instance)

Manager* Manager::self()
{
  return instance;
}

Manager::Manager(QObject* parent) : QObject(parent),
d_ptr(new GameManagerPrivate)
{
  
  kDebug() << "creating a GameManager";
}

Manager::~Manager()
{
  kDebug() << " !!! ----- Destroying a Game Manager ----- !!!";
}


void Manager::startTime()
{
    Q_D(GameManager);
    if ( !d->running )
    {
        d->timer = startTimer ( TimerInterval );
        d->running = true;
    }
}

void Manager::stopTime()
{
    Q_D(GameManager);
    if ( d->running )
    {
        killTimer(d->timer);
        d->running = false;
    }
}

void Manager::setCurrentTime(Color color, const QTime& time)
{
    Q_D(GameManager);
    switch ( color )
    {
        case White:
            d->whiteTimeControl.currentTime = time;
            break;
        case Black:
            d->blackTimeControl.currentTime = time;
            break;
        default:
            return;
    }
    emit timeChanged ( color, time );
}

    void Manager::timerEvent(QTimerEvent* )
    {
        Q_D(GameManager);
        QTime time;
        switch ( d->activePlayer )
        {
            case White:
                d->whiteTimeControl.currentTime = d->whiteTimeControl.currentTime.addMSecs ( -TimerInterval );
                time = d->whiteTimeControl.currentTime;
                break;
            case Black:
                d->blackTimeControl.currentTime = d->blackTimeControl.currentTime.addMSecs ( -TimerInterval );
                time = d->blackTimeControl.currentTime;
                break;
            default:
                time = QTime();
                break;
        }
        emit timeChanged ( d->activePlayer, time );
    }

     void Manager::addMoveToHistory ( const Move& move )
    {
        Q_D ( GameManager );
        if ( d->moveHistory.isEmpty() )
        {
            emit undoPossible(true);
        }
        d->moveHistory << move;
        if ( !d->moveUndoStack.isEmpty() )
        {
            emit redoPossible(false);
        }
        d->moveUndoStack.clear();
    }

    Move Manager::nextUndoMove()
    {
        Q_D ( GameManager );
        Move m = d->moveHistory.pop();
        if ( d->moveHistory.isEmpty() )
        {
            emit undoPossible(false);
        }
        if ( d->moveUndoStack.isEmpty() )
        {
            emit redoPossible(true);
        }
        d->moveUndoStack.push( m );
        Move ret = m.reverse();
        ret.setFlag ( Move::Forced, true );
        return ret;
    }

    Move Manager::nextRedoMove()
    {
        Q_D ( GameManager );
        Move m = d->moveUndoStack.pop();
        if ( d->moveUndoStack.isEmpty() )
        {
            emit redoPossible(false);
        }
        if ( d->moveHistory.isEmpty() )
        {
            emit undoPossible(true);
        }
        d->moveHistory << m;
        m.setFlag ( Move::Forced, true );
        return m;
    }


void Manager::setActivePlayer(Color player)
{
    Q_D(GameManager);
    d->activePlayer = player;
}

void Manager::changeActivePlayer()
{
    setActivePlayer ( oppositeColor ( activePlayer() ) );
    emit activePlayerChanged ( activePlayer() );
}

Color Manager::activePlayer() const
{
    Q_D(const GameManager);
    return d->activePlayer;
}

void Manager::initialize()
{
  Q_D(GameManager);
  d->running = false;
  d->activePlayer = White;
  d->whiteTimeControl.currentTime = d->whiteTimeControl.baseTime;
  d->blackTimeControl.currentTime = d->blackTimeControl.baseTime;
  Protocol::white()->setTimeControl(d->whiteTimeControl);
  Protocol::black()->setTimeControl(d->blackTimeControl);
  connect ( Protocol::white(), SIGNAL(pieceMoved(Move)), SLOT(moveByProtocol(Move)) );
  connect ( Protocol::black(), SIGNAL(pieceMoved(Move)), SLOT(moveByProtocol(Move)) );
  Protocol::white()->init();
  Protocol::black()->init();
}

void Manager::setTimeControl(Color color, const Knights::TimeControl& control)
{
  Q_D(GameManager);
  if ( color == White )
  {
    d->whiteTimeControl = control;
  }
  else if ( color == Black )
  {
    d->blackTimeControl = control;
  }
  else
  {
    kDebug() << "Setting time control fo NoColor";
    d->blackTimeControl = control;
    d->whiteTimeControl = control;
  }
}

TimeControl Manager::timeControl(Color color) const
{
  Q_D(const GameManager);
  if ( color == White )
  {
    return d->whiteTimeControl;
  }
  else if ( color == Black )
  {
    return d->blackTimeControl;
  }
  else
  {
    return TimeControl();
  }
}

QTime Manager::timeLimit(Color color)
{
  return timeControl(color).baseTime;
}

bool Manager::timeControlEnabled(Color color) const
{
  return timeControl(color).baseTime.isValid();
}

void Manager::undo()
{
  emit pieceMoved ( nextUndoMove() );
}

void Manager::redo()
{
  emit pieceMoved ( nextRedoMove() );
}

bool Manager::isRunning()
{
  Q_D(const GameManager);
  return d->running;
}

void Manager::moveByProtocol(const Knights::Move& move)
{
  Q_D(GameManager);
  if ( sender() != Protocol::byColor ( d->activePlayer ) )
  {
    // Ignore duplicates and/or moves by the inactive player
    return;
  }
  d->moveHistory << move;
  Protocol::byColor ( oppositeColor ( d->activePlayer ) )->move(move);
  emit pieceMoved(move);
  changeActivePlayer();
}

void Manager::moveByBoard(const Knights::Move& move)
{
  Q_D(GameManager);
  if ( !Protocol::byColor(d->activePlayer)->isLocal() )
  {
    // Only local protocols can make moves from the board
    return;
  }
  Protocol::byColor ( oppositeColor ( d->activePlayer ) )->move(move);
  changeActivePlayer();
}






