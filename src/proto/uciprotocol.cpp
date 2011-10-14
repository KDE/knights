#include "uciprotocol.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>

using namespace Knights;

UciProtocol::UciProtocol(QObject* parent): TextProtocol(parent)
{

}

UciProtocol::~UciProtocol()
{

}

bool UciProtocol::parseStub(const QString& line)
{
  return false;
}

void UciProtocol::parseLine(const QString& line)
{
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
  QString str;
  str += m.from().string() += m.to().string();
  if ( m.promotedType() )
  {
    str += Piece::charFromType ( m.promotedType() ).toLower();
  }
  
  write ( str );
}

