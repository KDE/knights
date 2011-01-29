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
#include <KLocale>
#include "rules/rules.h"
#include "rules/chessrules.h"

using namespace Knights;

const int TimerInterval = 100;

class Knights::GameManagerPrivate
{
public:
  GameManagerPrivate();
  
  Color activePlayer;
  bool running;
  bool gameStarted;
  int timer;

  TimeControl whiteTimeControl;
  TimeControl blackTimeControl;

  QStack<Move> moveHistory;
  QStack<Move> moveUndoStack;

  Rules* rules;

  QMap<int, Offer> offers;
};

GameManagerPrivate::GameManagerPrivate()
  : activePlayer(NoColor),
    running(false),
    gameStarted(false),
    timer(0),
    rules(0)
{

}

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
  delete d_ptr;
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

void Manager::setTimeRunning(bool running)
{
  if ( running )
  {
    startTime();
  }
  else
  {
    stopTime();
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
	kDebug() << d->moveHistory;
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
	kDebug() << m << ret;
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
  d->gameStarted = false;
  d->running = false;
  d->activePlayer = White;
  d->whiteTimeControl.currentTime = d->whiteTimeControl.baseTime;
  d->blackTimeControl.currentTime = d->blackTimeControl.baseTime;
  Protocol::white()->setTimeControl(d->whiteTimeControl);
  Protocol::black()->setTimeControl(d->blackTimeControl);
  connect ( Protocol::white(), SIGNAL(pieceMoved(Move)), SLOT(moveByProtocol(Move)) );
  connect ( Protocol::white(), SIGNAL(initSuccesful()), SLOT(protocolInitSuccesful()), Qt::QueuedConnection );
  connect ( Protocol::black(), SIGNAL(pieceMoved(Move)), SLOT(moveByProtocol(Move)) );
  connect ( Protocol::black(), SIGNAL(initSuccesful()), SLOT(protocolInitSuccesful()), Qt::QueuedConnection );
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
    kDebug() << "Setting time control for both colors";
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
}

void Manager::redo()
{
  Move m = nextRedoMove();
  Protocol::white()->move ( m );
  Protocol::black()->move ( m );
  emit pieceMoved ( m );
  changeActivePlayer();
}

void Manager::resign()
{
  Q_D(const GameManager);
  //TODO:
}

bool Manager::isRunning()
{
  Q_D(const GameManager);
  return d->running;
}

void Manager::moveByProtocol(const Move& move)
{
  Q_D(GameManager);
  if ( sender() != Protocol::byColor ( d->activePlayer ) )
  {
    kDebug() << "Move by the non-active player" << move;
    // Ignore duplicates and/or moves by the inactive player
    return;
  }
  Move m = move;
  d->rules->checkSpecialFlags ( &m, d->activePlayer );
  if ( m.flag(Move::Illegal) && !m.flag(Move::Forced) )
  {
    // Most likely happens in a local game, when both protocols report the same move from the board.
    return;
  }
  kDebug() << "Adding move" << m << "to history";
  addMoveToHistory ( m );
  Protocol::byColor ( oppositeColor ( d->activePlayer ) )->move ( m );
  emit pieceMoved ( m );
  rules()->moveMade ( m );
  changeActivePlayer();
}

void Manager::moveByBoard(const Move& move)
{
  Q_D(GameManager);
  if ( !Protocol::byColor(d->activePlayer)->isLocal() )
  {
    // Only local protocols can make moves from the board
    return;
  }
  Move m = move;
  d->rules->checkSpecialFlags ( &m, d->activePlayer );
  Protocol::byColor ( oppositeColor ( d->activePlayer ) )->move ( m );
  d->rules->moveMade ( m );
  addMoveToHistory ( m );
  changeActivePlayer();
}

void Manager::protocolInitSuccesful()
{
  Q_D(GameManager);
  if ( !d->gameStarted && Protocol::white()->isReady() && Protocol::black()->isReady() )
  {
    emit initComplete();
    d->rules = new ChessRules();
    Protocol::white()->startGame();
    Protocol::black()->startGame();
    d->gameStarted = true;
  }
}

void Manager::gameOver()
{
  Q_D(GameManager);
  stopTime();
  if ( d->gameStarted )
  {
    delete Protocol::white();
    delete Protocol::black();
    delete d->rules;
  }
  d->gameStarted = false;
}

Rules* Manager::rules()
{
  Q_D(const GameManager);
  return d->rules;
}

void Manager::sendOffer(const Offer& offer)
{
  Q_D(GameManager);
  Offer o = offer;
  QString name = Protocol::byColor(offer.player)->playerName();
  switch ( offer.action )
  {
    case ActionDraw:
      o.text = i18n("%1 offers you a draw", name);
      break;
    case ActionUndo:
      o.text = i18np("%2 would like to take back a half move", "%2 would like to take back %1 half moves", o.numberOfMoves, name);
      break;
    case ActionAdjourn:
      o.text = i18n("%1 would like to adjourn the game", o.player);
      break;
    case ActionAbort:
      o.text = i18n("%1 would like to abort the game");
      break;
    default:
      break;
  }
  d->offers.insert ( o.id, o );
  Protocol* opp = Protocol::byColor( oppositeColor(o.player) );
  if ( opp->isLocal() )
  {
    emit notification(offer);
  }
  else
  {
    opp->makeOffer(o);
  }
}

void Manager::setOfferResult(int id, OfferAction result)
{
  Q_D(GameManager);
  if ( result == AcceptOffer )
  {  
      Protocol::byColor(d->offers[id].player)->acceptOffer(id);
      switch ( d->offers[id].action )
      {
	case ActionUndo:
	  emit pieceMoved ( nextUndoMove() );
	  changeActivePlayer();
	  break;
	  
	case ActionDraw:
	  Protocol::white()->setWinner(NoColor);
	  Protocol::black()->setWinner(NoColor);
	  break;
	  
	case ActionAbort:
	  gameOver();
	  break;
	  
	case ActionPause:
	  stopTime();
	  break;
	  
	case ActionResume:
	  startTime();
	  break;
	  
	default:
	  break;
      }
  }
   else if ( result == DeclineOffer )
   { 
     Protocol::byColor(d->offers[id].player)->declineOffer(id);
  }
  d->offers.remove(id);
}









