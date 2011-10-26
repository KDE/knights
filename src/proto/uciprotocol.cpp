/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2011  Miha Čančula <miha@noughmad.eu>

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

#include "uciprotocol.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>
#include "gamemanager.h"

using namespace Knights;

UciProtocol::UciProtocol(QObject* parent): ComputerProtocol(parent)
{

}

UciProtocol::~UciProtocol()
{
    if ( mProcess && mProcess->isOpen() )
    {
        write("quit");
        if ( !mProcess->waitForFinished ( 500 ) )
        {
            mProcess->kill();
        }
    }
}

Protocol::Features UciProtocol::supportedFeatures()
{
    return GameOver | Pause | Draw | Adjourn | Resign | Undo | SetDifficulty | AdjustDifficulty;
}

bool UciProtocol::parseStub(const QString& line)
{
  Q_UNUSED(line);
  return false;
}

void UciProtocol::parseLine(const QString& line)
{
  kDebug() << line;
  if ( line.startsWith ( QLatin1String("uciok") ) )
  {
    write ( "isready" );
  }
  else if ( line.startsWith ( QLatin1String("readyok") ) )
  {
    emit initComplete();
  }
  else if ( line.startsWith ( QLatin1String("id name ") ) )
  {
    // Chop off the "id name " port, the remainder if the engine's name
    setPlayerName ( line.mid ( 8 ) );
  }
  else if ( line.startsWith ( QLatin1String("bestmove") ) )
  {
    kDebug() << line;
    QStringList lst = line.split(QLatin1Char(' '));
    if ( lst.size() > 1 )
    {
      Move m = Move ( lst[1] );
      mMoveHistory << m;
      emit pieceMoved ( m );
    }
    if ( lst.size() > 3 && lst[2] == QLatin1String("ponder") )
    {
      mPonderMove.setString ( lst[3] );      
    }
  }
}

void UciProtocol::declineOffer(const Knights::Offer& offer)
{

}

void UciProtocol::acceptOffer(const Knights::Offer& offer)
{

}

void UciProtocol::makeOffer(const Knights::Offer& offer)
{

}

void UciProtocol::startGame()
{
  write ( "ucinewgame" );
}

void UciProtocol::init()
{
    startProgram();
    write("uci");
}

void UciProtocol::move(const Knights::Move& m)
{
  mMoveHistory << m;
  
  QString str = QLatin1String("position startpos moves ");
  foreach ( const Move& move, mMoveHistory )
  { 
    QString moveString = move.from().string() + move.to().string();
    if ( m.promotedType() )
    {
      moveString += Piece::charFromType ( move.promotedType() ).toLower();
    }
    str += moveString += QLatin1Char(' ');
  }
  write ( str );
  
  QString goString = QLatin1String("go");
  
  if ( mDifficulty )
  {
    goString += QLatin1String(" depth ") + QString::number ( mDifficulty );
  }

  if ( Manager::self()->timeControlEnabled(NoColor) )
  {
    goString += QLatin1String(" wtime ") + QString::number ( mWhiteTime );
    goString += QLatin1String(" btime ") + QString::number ( mBlackTime );
    
    int winc = Manager::self()->timeControl ( White ).increment;
    if ( winc )
    {
      goString += QLatin1String(" winc ") + QString::number ( winc * 1000 );
    }
    int binc = Manager::self()->timeControl ( Black ).increment;
    if ( winc )
    {
      goString += QLatin1String(" binc ") + QString::number ( binc * 1000 );
    }
    
    int moves = Manager::self()->timeControl ( NoColor ).moves;
    int movesToGo = mMoveHistory.size() % moves;
    if ( movesToGo > 0 )
    {
      goString += QLatin1String(" movestogo ") + QString::number ( movesToGo );
    }
  }
  
  write ( goString );
}

void UciProtocol::changeCurrentTime(Color color, const QTime& time)
{
  int msecs = QTime().msecsTo ( time );
  switch ( color )
  {
    case White:
      mWhiteTime = msecs;
      break;
      
    case Black:
      mBlackTime = msecs;
      break;
      
    default:
      break;
  }
}

void UciProtocol::setDifficulty(int level)
{
    mDifficulty = level;
}


