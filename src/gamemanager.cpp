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

#include "gamemanager.h"

#include "core/move.h"

#include <QStack>
#include "proto/protocol.h"
#include <KDebug>
#include <KLocale>
#include "rules/rules.h"
#include "rules/chessrules.h"
#include <QTimer>
#include <settings.h>

using namespace Knights;

const int TimerInterval = 100;

void Offer::accept() const
{
  Manager::self()->setOfferResult(id, AcceptOffer);
}

void Offer::decline() const
{
  Manager::self()->setOfferResult(id, DeclineOffer);
}

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
		if ( QTime().msecsTo(d->whiteTimeControl.currentTime) < TimerInterval )
		{
		  gameOver(Black);
		}
                d->whiteTimeControl.currentTime = d->whiteTimeControl.currentTime.addMSecs ( -TimerInterval );
                time = d->whiteTimeControl.currentTime;
                break;
            case Black:
		if ( QTime().msecsTo(d->blackTimeControl.currentTime) < TimerInterval )
		{
		  gameOver(White);
		}
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
	if ( m.pieceData().first == White )
	{
	  d->whiteTimeControl.currentTime = m.time();
	}
	else
	{
	  d->blackTimeControl.currentTime = m.time();
	}
	emit timeChanged ( m.pieceData().first, m.time() );
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
  d->gameStarted = false;
  d->running = false;
  d->activePlayer = White;
  d->whiteTimeControl.currentTime = d->whiteTimeControl.baseTime;
  d->blackTimeControl.currentTime = d->blackTimeControl.baseTime;
  QList<Protocol*> protocols;
  Protocol::white()->setTimeControl(d->whiteTimeControl);
  Protocol::black()->setTimeControl(d->blackTimeControl);
  connect ( Protocol::white(), SIGNAL(pieceMoved(Move)), SLOT(moveByProtocol(Move)) );
  connect ( Protocol::white(), SIGNAL(initSuccesful()), SLOT(protocolInitSuccesful()), Qt::QueuedConnection );
  connect ( Protocol::white(), SIGNAL(gameOver(Color)), SLOT(gameOver(Color)) );
  connect ( Protocol::black(), SIGNAL(pieceMoved(Move)), SLOT(moveByProtocol(Move)) );
  connect ( Protocol::black(), SIGNAL(initSuccesful()), SLOT(protocolInitSuccesful()), Qt::QueuedConnection );
  connect ( Protocol::black(), SIGNAL(gameOver(Color)), SLOT(gameOver(Color)) );
  Protocol::white()->init();
  Protocol::black()->init();
}

void Manager::setTimeControl(Color color, const TimeControl& control)
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
  sendPendingMove();
  Q_D(const GameManager);
  Offer o;
  o.action = ActionUndo;
  
  // We always undo moves until it's local player's turn again. 
  if ( Protocol::byColor(d->activePlayer)->isLocal() )
  {
    o.numberOfMoves = 2;
  }
  else
  {
    o.numberOfMoves = 1;
  }
  o.numberOfMoves = qMin ( o.numberOfMoves, d->moveHistory.size() );
  o.player = local()->color();
  sendOffer(o);
}

void Manager::redo()
{
  sendPendingMove();
  Move m = nextRedoMove();
  Protocol::white()->move ( m );
  Protocol::black()->move ( m );
  emit pieceMoved ( m );
  changeActivePlayer();
}

void Manager::adjourn()
{
  Offer o;
  o.action = ActionAdjourn;
  o.id = qrand();
  o.player = local()->color();
  sendOffer(o);
}

void Manager::offerDraw()
{
  Offer o;
  o.action = ActionDraw;
  o.id = qrand();
  o.player = local()->color();
  sendOffer(o);
}

void Manager::resign()
{
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
  if ( sender() != Protocol::byColor ( d->activePlayer ) || !d->gameStarted )
  {
    kDebug() << "Move by the non-active player" << move;
    // Ignore duplicates and/or moves by the inactive player
    return;
  }
  processMove(move);
}

void Manager::protocolInitSuccesful()
{
  Q_D(GameManager);
  if ( !d->gameStarted && Protocol::white()->isReady() && Protocol::black()->isReady() )
  {
    if ( Protocol::white()->isLocal() && Protocol::black()->isLocal() )
    {
      Protocol::white()->setPlayerName ( i18nc ( "The player of this color", "White" ) );
      Protocol::black()->setPlayerName ( i18nc ( "The player of this color", "Black" ) );
    }
    emit initComplete();
  }
}

void Manager::startGame()
{
    Q_D(GameManager);
    Protocol::white()->startGame();
    Protocol::black()->startGame();
    d->gameStarted = true;
}


void Manager::gameOver(Color winner)
{
  sendPendingMove();
  Q_D(const GameManager);
  if ( d->gameStarted )
  {
    stopTime();
    Protocol::white()->setWinner(winner);
    Protocol::black()->setWinner(winner);
    emit winnerNotify(winner);
    reset();
  }
}

void Manager::reset()
{
  Q_D(GameManager);
  stopTime();
  if ( d->gameStarted )
  {
    if ( Protocol::white() )
    {
      Protocol::white()->deleteLater();
    }
    if ( Protocol::black() )
    {
      Protocol::black()->deleteLater();
    }
    delete d->rules;
  }
  d->moveHistory.clear();
  d->moveUndoStack.clear();
  d->gameStarted = false;
}

Rules* Manager::rules() const
{
  Q_D(const GameManager);
  return d->rules;
}

void Manager::setRules(Rules* rules)
{
  Q_D(GameManager);
  d->rules = rules;
}

void Manager::sendOffer(const Offer& offer)
{
  Q_D(GameManager);
  Offer o = offer;
  if ( offer.player == NoColor )
  {
      o.player = local()->color();
  }
  QString name = Protocol::byColor(o.player)->playerName();
  if ( o.text.isEmpty() )
  {
    switch ( offer.action )
    {
      case ActionDraw:
	o.text = i18n("%1 offers you a draw", name);
	break;
      case ActionUndo:
	o.text = i18np("%2 would like to take back a half move", "%2 would like to take back %1 half moves", o.numberOfMoves, name);
	break;
      case ActionAdjourn:
	o.text = i18n("%1 would like to adjourn the game", name);
	break;
      case ActionAbort:
	o.text = i18n("%1 would like to abort the game", name);
	break;
      default:
	break;
    }
  }
  d->offers.insert ( o.id, o );
  Protocol* opp = Protocol::byColor( oppositeColor(o.player) );
  // Only display a notification if only one player is local.
  if ( opp->isLocal() && !Protocol::byColor(o.player)->isLocal() )
  {
    emit notification(o);
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
      Protocol::byColor(d->offers[id].player)->acceptOffer(d->offers[id]);
      switch ( d->offers[id].action )
      {
	case ActionUndo:
	  for ( int i = 0; i < d->offers[id].numberOfMoves; ++i )
	  {
	    emit pieceMoved ( nextUndoMove() );
	    changeActivePlayer();
	  }
	  break;
	  
	case ActionDraw:
	  Protocol::white()->setWinner(NoColor);
	  Protocol::black()->setWinner(NoColor);
	  break;
	  
	case ActionAbort:
	  gameOver(NoColor);
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
     Protocol::byColor(d->offers[id].player)->declineOffer(d->offers[id]);
  }
  d->offers.remove(id);
}

Protocol* Manager::local()
{
  Q_D(const GameManager);
  if ( Protocol::byColor(d->activePlayer)->isLocal() )
  {
    return Protocol::byColor(d->activePlayer);
  }
  if ( Protocol::byColor(oppositeColor(d->activePlayer))->isLocal() )
  {
    return Protocol::byColor(oppositeColor(d->activePlayer));
  }
  kWarning() << "No local protocols, trying a computer";
  if ( Protocol::byColor(d->activePlayer)->isComputer() )
  {
    return Protocol::byColor(d->activePlayer);
  }
  if ( Protocol::byColor(oppositeColor(d->activePlayer))->isComputer() )
  {
    return Protocol::byColor(oppositeColor(d->activePlayer));
  }
  kWarning() << "No local or computer protocols, returning 0";
  return 0;
}

bool Manager::canRedo() const
{
  Q_D(const GameManager);
  return !d->moveUndoStack.isEmpty();
}

void Manager::sendPendingMove()
{
  if ( pendingMove.isValid() )
  {
    Q_D(GameManager);
    Protocol::byColor ( oppositeColor ( d->activePlayer ) )->move ( pendingMove );
    emit pieceMoved ( pendingMove );
    rules()->moveMade ( pendingMove );
    pendingMove = Move();
    
    Color winner = rules()->winner();
    if ( winner != NoColor || !rules()->hasLegalMoves ( oppositeColor( d->activePlayer ) ) )
    {
        kDebug() << "Winner: " << winner;
        gameOver ( winner );
    }
    
    int moveNumber;
    int secondsAdded = 0;
    switch ( d->activePlayer )
    {
      case White:
	moveNumber = ( d->moveHistory.size() + 1 ) / 2;
	if ( moveNumber > 1 )
	{
	  secondsAdded += d->whiteTimeControl.increment;
	}
	if ( d->whiteTimeControl.moves > 0 && ( moveNumber % d->whiteTimeControl.moves ) == 0 )
	{
	  secondsAdded += QTime().secsTo( d->whiteTimeControl.baseTime );
	}
	if ( secondsAdded != 0 )
	{
	  setCurrentTime ( White, d->whiteTimeControl.currentTime.addSecs ( secondsAdded ) );
	}
	break;
	
      case Black:
	moveNumber = d->moveHistory.size() / 2;
	if ( moveNumber > 1 )
	{
	  secondsAdded += d->blackTimeControl.increment;
	}
	if ( d->blackTimeControl.moves > 0 && ( moveNumber % d->blackTimeControl.moves ) == 0 )
	{
	  secondsAdded += QTime().secsTo ( d->blackTimeControl.baseTime );
	}
	if ( secondsAdded != 0 )
	{
	  setCurrentTime ( Black, d->blackTimeControl.currentTime.addSecs ( secondsAdded ) );
	}
	break;
	
      default:
	break;
    }
    changeActivePlayer();
  }
}

void Manager::moveByBoard(const Move& move)
{
  processMove(move);
}

void Manager::processMove(const Move& move)
{
  Q_D(const GameManager);
  Move m = move;
  if ( activePlayer() == White )
  {
    m.setTime ( d->whiteTimeControl.currentTime );
  }
  else
  {
    m.setTime ( d->blackTimeControl.currentTime );
  }
  d->rules->checkSpecialFlags ( &m, d->activePlayer );
  if ( m.flag(Move::Illegal) && !m.flag(Move::Forced) )
  {
    return;
  }
  addMoveToHistory ( m );
  if ( d->moveHistory.size() == 2 && timeControlEnabled(d->activePlayer) )
  {
    startTime();
  }
  pendingMove = m;
  if ( Protocol::byColor(d->activePlayer)->isComputer() )
  {
    QTimer::singleShot ( Settings::computerDelay(), this, SLOT(sendPendingMove()) );
  }
  else
  {
    sendPendingMove();
  }
}
