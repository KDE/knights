#include "uciprotocol.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>
#include "gamemanager.h"

using namespace Knights;

UciProtocol::UciProtocol(QObject* parent): TextProtocol(parent)
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
    QStringList args = attribute("program").toString().split ( QLatin1Char ( ' ' ) );
    QString program = args.takeFirst();
    setPlayerName ( program );
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    mProcess->setReadChannel ( KProcess::StandardOutput );
    connect ( mProcess, SIGNAL (readyReadStandardError()), SLOT (readError()) );
    setDevice ( mProcess );
    kDebug() << "Starting program" << program << "with args" << args;
    mProcess->start();
    if ( !mProcess->waitForStarted ( 1000 ) )
    {
        emit error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it is installed.", program ) );
        return;
    }
    
    // TODO: Write options and register here
    
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
  
  QString goString = QLatin1String("go depth 4"); // TODO: Change the magic number, this is basically difficulty
  
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

