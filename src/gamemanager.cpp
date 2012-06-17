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
#include "proto/protocol.h"
#include "rules/rules.h"
#include "rules/chessrules.h"
#include "externalcontrol.h"
#include "settings.h"
#include "ui_customdifficultydialog.h"

#include <KDebug>
#include <KSpeech>
#include "kspeechinterface.h"
#include <KFileDialog>
#include <KLocale>
#include <KSaveFile>
#include <KMessageBox>
#include <KStandardGuiItem>
#include <KApplication>
#include <KgDifficulty>

#include <QStack>
#include <QTimer>
#include <QTextStream>
#include <QStringListModel>

using namespace Knights;

const int TimerInterval = 100;
const int LineLimit = 80; // Maximum characters per line for PGN format

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
  QSet<int> usedOfferIds;
  
  org::kde::KSpeech* speech;
  ExternalControl* extControl;
  
  QString filename;
   Color winner;
   bool winnerNotified;
   bool initComplete;
  
  int nextOfferId();
};

GameManagerPrivate::GameManagerPrivate()
  : activePlayer(NoColor),
    running(false),
    gameStarted(false),
    timer(0),
    rules(0),
    speech(0),
    extControl(0)
{

}

int GameManagerPrivate::nextOfferId()
{
  int i = usedOfferIds.size() + 1;
  while (usedOfferIds.contains(i))
  {
    ++i;
  }
  return i;
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
  Q_D(GameManager);
  d->speech = new org::kde::KSpeech(
      QLatin1String("org.kde.kttsd"), 
                    QLatin1String("/KSpeech"),
                    QDBusConnection::sessionBus()
      );
  d->speech->setApplicationName(qAppName());
}

Manager::~Manager()
{
  kDebug() << " !!! ----- Destroying a Game Manager ----- !!!";
  Q_D(GameManager);
  delete d->speech;
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
        emit historyChanged();
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

        emit historyChanged();
        
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
        emit historyChanged();
        
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
  d->initComplete = false;
  d->winnerNotified = false;
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
  d->extControl = new ExternalControl(this);
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
    // FICS protocol needs the time control parameters even before the color is determined
    // It only supports equal time for both players, so it doesn't matter which one we return
    return d->whiteTimeControl;
  }
}

QTime Manager::timeLimit(Color color)
{
  return timeControl(color).baseTime;
}

bool Manager::timeControlEnabled(Color color) const
{
  TimeControl tc = timeControl(color);
  
  // For a time to be valid, either the base time or increment must be greater than 0
  if ( QTime().secsTo(tc.baseTime) > 0 || tc.increment > 0 )
  {
    return true;
  }
  return false;
}

void Manager::undo()
{
  sendPendingMove();
  Q_D(const GameManager);
  Offer o;
  o.action = ActionUndo;
  
  // We always undo moves until it's local player's turn again. 
  if ( Protocol::byColor(d->activePlayer)->isLocal() && !Protocol::byColor(oppositeColor(d->activePlayer))->isLocal() )
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
  sendOffer(ActionAdjourn);
}

void Manager::abort()
{
  sendOffer(ActionAbort);
}

void Manager::offerDraw()
{
  sendOffer(ActionDraw);
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
  if ( Protocol::white() && Protocol::black() )
  {
    if ( !d->gameStarted && Protocol::white()->isReady() && Protocol::black()->isReady() )
    {
      if ( Protocol::white()->isLocal() && Protocol::black()->isLocal() )
      {
        Protocol::white()->setPlayerName ( i18nc ( "The player of this color", "White" ) );
        Protocol::black()->setPlayerName ( i18nc ( "The player of this color", "Black" ) );
      }
      if (!d->initComplete)
      {
        d->initComplete = true;
        emit initComplete();
      }
    }
  }
}

void Manager::startGame()
{
    Q_D(GameManager);
    Q_ASSERT(!d->gameStarted);
    levelChanged ( Kg::difficulty()->currentLevel() );
    Protocol::white()->startGame();
    Protocol::black()->startGame();
    d->gameStarted = true;
    emit historyChanged();
}


void Manager::gameOver(Color winner)
{
  sendPendingMove();
  Q_D(GameManager);
  if ( d->gameStarted )
  {
    stopTime();
    if ( !d->winnerNotified )
    {
      d->winner = winner;
      Protocol::white()->setWinner(winner);
      Protocol::black()->setWinner(winner);
      emit winnerNotify(winner);
      
      d->winnerNotified = true;
    }
  }
}

void Manager::reset()
{
  Q_D(GameManager);
  sendPendingMove();
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
  emit undoPossible( false );
  emit redoPossible( false );
  
  d->offers.clear();
  d->usedOfferIds.clear();
  
  d->gameStarted = false;
  d->winner = NoColor;
  d->winnerNotified = false;
  
  if ( d->extControl )
  {
    delete d->extControl;
    d->extControl = 0;
  }
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

void Manager::sendOffer(GameAction action, Color player, int id)
{
    Offer o;
    o.action = action;
    o.player = player;
    o.id = id;
    sendOffer(o);
}


void Manager::sendOffer(const Offer& offer)
{
  Q_D(GameManager);
  Offer o = offer;
  if ( offer.player == NoColor )
  {
      o.player = local()->color();
  }
  
  if (o.id == 0)
  {
    o.id = d->nextOfferId();
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
  d->usedOfferIds << o.id;
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
  if ( pendingMove.isValid() && isGameActive() )
  {
    Q_D(GameManager);
    Protocol::byColor ( oppositeColor ( d->activePlayer ) )->move ( pendingMove );
    emit pieceMoved ( pendingMove );
    rules()->moveMade ( pendingMove );
    
    if ( Settings::speakOpponentsMoves() 
        && !Protocol::byColor(d->activePlayer)->isLocal()
        && Protocol::byColor(oppositeColor(d->activePlayer))->isLocal() )
    {
        QString toSpeak;
        QString name = Protocol::byColor(d->activePlayer)->playerName();
        if ( pendingMove.flag(Move::Castle) )
        {
            if ( pendingMove.to().first == 3 )
            {
                toSpeak = i18nc("string to be spoken when the opponent castles queenside",
                    "%1 castles queenside", name);
            }
            else
            {
                toSpeak = i18nc("string to be spoken when the opponent castles queenside",
                               "%1 castles kingside", name);
            }
        }
        else
        {
            toSpeak = i18nc("string to be spoken when the opponent makes a normal  move",
                            "%1 to %2",
                            pieceTypeName ( pendingMove.pieceData().second ),
                            pendingMove.to().string()
            );
        }
        kDebug() << toSpeak;
        d->speech->say(toSpeak, KSpeech::soPlainText);
        
        if ( pendingMove.flag(Move::Check) )
        {
            if ( d->rules->hasLegalMoves ( oppositeColor( d->activePlayer ) ) )
            {
                d->speech->say ( i18nc( "Your king is under attack", "Check" ), KSpeech::soPlainText );
            }
            else
            {
                d->speech->say ( i18nc( "Your king is dead", "Checkmate" ), KSpeech::soPlainText );
            }
        }
    }
    
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

void Manager::moveByExternalControl(const Knights::Move& move)
{
    Q_D(GameManager);
    if ( Settings::allowExternalControl() && Protocol::byColor(d->activePlayer)->isLocal() )
    {
        processMove(move);
    }
}


void Manager::processMove(const Move& move)
{
  sendPendingMove();
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

bool Manager::isGameActive() const
{
  Q_D(const GameManager);
  return d->gameStarted;
}

bool Manager::canLocalMove() const
{
  Q_D(const GameManager);
  if ( !d->gameStarted )
  {
    return false;
  }
  if ( d->running || d->moveHistory.size() < 2 || !timeControlEnabled(NoColor) )
  {
    return Protocol::byColor(d->activePlayer)->isLocal();
  }
  return false;
}

void Manager::levelChanged ( const KgDifficultyLevel* level )
{
  kDebug();
  int depth = 0;
  int size = 32;
  switch ( level->standardLevel() )
  {
    case KgDifficultyLevel::VeryEasy:
      depth = 1;
      break;
      
    case KgDifficultyLevel::Easy:
      depth = 3;
      break;
      
    case KgDifficultyLevel::Medium:
      depth = 8;
      break;
      
    case KgDifficultyLevel::Hard:
      depth = 16;
      break;
      
    case KgDifficultyLevel::VeryHard:
      depth = 32;
      break;
      
    case KgDifficultyLevel::Custom:
      // Open the dialog for the user to specify custom difficulty parameters
      if ( !getCustomDifficulty(&depth, &size) )
      {
        return;
      }
      break;
      
    default: 
      break;
  }
  
  setDifficulty ( depth, size );
}

void Manager::setDifficulty(int searchDepth, int memorySize)
{
  foreach ( Protocol* p, QList<Protocol*>() << Protocol::white() << Protocol::black() )
  {
    if ( p && p->supportedFeatures() & Protocol::AdjustDifficulty )
    {
      p->setDifficulty ( searchDepth, memorySize );
    }
  }
}

bool Manager::getCustomDifficulty(int* depth, int* size)
{
  bool accepted = false;
  QPointer<KDialog> dialog = new KDialog();
  QWidget* widget = new QWidget ( dialog );
  Ui::CustomDifficultyDialog* ui = new Ui::CustomDifficultyDialog;
  ui->setupUi ( widget );
  ui->searchDepthIntSpinBox->setSuffix ( ki18np(" move", " moves") );
  ui->memorySizeIntSpinBox->setValue ( Settings::computerMemorySize() );
  ui->searchDepthIntSpinBox->setValue ( Settings::computerSearchDepth() );
  dialog->setMainWidget ( widget );
  if ( dialog->exec() == KDialog::Accepted )
  {
    accepted = true;
    *depth = ui->searchDepthIntSpinBox->value();
    Settings::setComputerSearchDepth ( ui->searchDepthIntSpinBox->value() );
    *size = ui->memorySizeIntSpinBox->value();
    Settings::setComputerMemorySize ( ui->memorySizeIntSpinBox->value() );
  }
  delete dialog;
  delete ui;
  return accepted;
}

void Manager::loadGameHistoryFrom(const QString& filename)
{
  kDebug() << filename;
  QFile file(filename);
  if ( !file.open(QIODevice::ReadOnly) )
  {
    return;
  }
      
  QRegExp tagPairExp = QRegExp(QLatin1String( "\\[(.*)\\s\\\"(.*)\\\"\\]" ));
  while ( file.bytesAvailable() > 0 )
  {
    QByteArray line = file.readLine();
    if ( tagPairExp.indexIn ( QLatin1String(line) ) > -1 )
    {
      // Parse a tag pair
      QString key = tagPairExp.cap(1);
      QString value = tagPairExp.cap(2);
      
      if ( key == QLatin1String("White") )
      {
	Protocol::white()->setPlayerName ( value );
      }
      else if ( key == QLatin1String("Black") )
      {
	Protocol::black()->setPlayerName ( value );
      }
      else if ( key == QLatin1String("TimeControl") )
      {
	// TODO, optional: Parse TimeControl Tag
      }
    }
    else
    {
      // Parse a line of moves
      foreach ( const QByteArray& str, line.trimmed().split(' ') )
      {
	if ( !str.trimmed().isEmpty() && !str.contains('.') && !str.contains("1-0") && !str.contains("0-1") && !str.contains("1/2-1/2") && !str.contains('*') )
	{
	  // Only move numbers contain dots, not move data itself
	  // We also exclude the game termination markers (results)
	  kDebug() << "Read move" << str;
	  Move m;
          if (str.contains("O-O-O") || str.contains("o-o-o") || str.contains("0-0-0"))
          {
            m = Move::castling(Move::QueenSide, activePlayer());
          }
          else if (str.contains("O-O") || str.contains("o-o") || str.contains("0-0"))
          {
            m = Move::castling(Move::KingSide, activePlayer());
          }
          else
          {
            m = Move ( QLatin1String(str) );
          }
          m.setFlag ( Move::Forced, true );
	  processMove ( m );
	}
      }
    }
  }
  
  emit playerNameChanged();
}

void Manager::saveGameHistoryAs(const QString& filename)
{
  Q_D(GameManager);
  
  d->filename = filename;
  
  QFile file ( d->filename );
  file.open(QIODevice::WriteOnly);
  QTextStream stream ( &file );
  
  // Write the player tags first
  
  // Standard Tag Roster: Event, Site, Date, Round, White, Black, Result
  
  stream << "[Event \"Casual Game\"]" << endl;
  stream << "[Site \"?\"]" << endl;
  stream << "[Date \"" << QDate::currentDate().toString( QLatin1String("yyyy.MM.dd") ) << "\"]" << endl;
  stream << "[Round \"-\"]" << endl;
  stream << "[White \"" << Protocol::white()->playerName() << "\"]" << endl;
  stream << "[Black \"" << Protocol::black()->playerName() << "\"]" << endl;

  QByteArray result;
  if ( d->running )
  {
    result += '*';
  }
  else
  {
    switch ( d->winner )
    {
      case White:
        result = "1-0";
        break;
      case Black:
        result = "0-1";
        break;
      default:
        result = "1/2-1/2";
        break;
    }
  }
  stream << "[Result \"" << result << "\"]" << endl;

  // Supplemental tags, ordered alphabetacally. 
  // Currently, only TimeControl is added
  
  stream << "[TimeControl \"";
  if ( timeControlEnabled ( NoColor ) )
  {
    // The PGN specification doesn't include a time control combination with both a number of moves 
    // and an increment per move defined, so we only output one of them
    // If the spec will ever be expanded, the two lines should be combined:
    // stream << tc.moves << '/' << QTime().secsTo ( tc.baseTime ) << '+' << tc.increment
  
    TimeControl tc = timeControl ( NoColor );
    if ( tc.moves )
    {
      stream << tc.moves << '/' << QTime().secsTo ( tc.baseTime );
    }
    else
    {
      stream << QTime().secsTo ( tc.baseTime ) << '+' << tc.increment;
    }
  }
  else
  {
    stream << '-';
  }
  stream << "\"]";    
  
  // A single newline separates the tag pairs from the movetext section
  stream << endl;
  
  kDebug() << "Starting to write movetext";
  
  int characters = 0;
  int n = d->moveHistory.size();
  for (int i = 0; i < n; ++i)
  {
    Move m = d->moveHistory[i];
    const QString moveString = m.stringForNotation ( Move::Algebraic );
    QString output;
    
    if ( i % 2 == 0 )
    {
      // White move
      output = QString::number(i/2+1) + QLatin1String(". ") + moveString;
    }
    else
    {
      // Black move
      output = moveString;
    }
    
    if ( characters + output.size() > LineLimit )
    {
      stream << endl;
      characters = 0;
    }
    
    if ( characters != 0 )
    {
      stream << QLatin1Char(' ');
    }
    
    stream << output;
    characters += output.size();
  }
  
  kDebug();
  
  stream << ' ' << result;
  
  stream << endl;
  stream.flush();
  
  kDebug() << "Saved";
}

QStack< Move > Manager::moveHistory() const
{
  Q_D(const GameManager);
  return d->moveHistory;
}

