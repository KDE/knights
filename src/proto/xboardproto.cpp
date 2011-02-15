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

#include "proto/xboardproto.h"
#include "proto/chatwidget.h"
#include "gamemanager.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>
#include <KFileDialog>
#include <settings.h>

using namespace Knights;

XBoardProtocol::XBoardProtocol ( QObject* parent ) : TextProtocol ( parent )
, mProcess(0)
, m_moves(0)
, m_increment(0)
, m_baseTime(0)
, m_timeLimit(0)
{

}

Protocol::Features XBoardProtocol::supportedFeatures()
{
    return GameOver | Draw | Adjourn | Resign | Undo | Pause;
}

XBoardProtocol::~XBoardProtocol()
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

bool XBoardProtocol::isComputer()
{
    return true;
}

void XBoardProtocol::startGame()
{
    kDebug() << colorName(color());
    TimeControl c = Manager::self()->timeControl ( White );
    if ( c.baseTime != QTime() )
    {
        write(QString(QLatin1String("level %1 %2 %3")).arg(c.moves).arg(QTime().secsTo(c.baseTime)/60).arg(c.increment));
    }
    if ( color() == White )
    {
        write("go");
    }
    resumePending = false;
}

void XBoardProtocol::move ( const Move& m )
{
    kDebug() << "Player's move:" << m.string(false);
    write ( m.string(false) );
    lastMoveString.clear();
    emit undoPossible ( false );
    if ( resumePending )
    {
        write("go");
        resumePending = false;
    }
}

void XBoardProtocol::init (  )
{
    QStringList args = attribute("program").toString().split ( QLatin1Char ( ' ' ) );
    QString program = args.takeFirst();
    setPlayerName ( program );
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    mProcess->setReadChannel ( KProcess::StandardOutput );
    connect ( mProcess, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    setDevice(mProcess);
    kDebug() << "Starting program" << program << "with args" << args;
    mProcess->start();
    if ( !mProcess->waitForStarted ( 1000 ) )
    {
        emit error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it is installed.", program ) );
        return;
    }
    write("xboard");
    initComplete();
}

QList< Protocol::ToolWidgetData > XBoardProtocol::toolWidgets()
{
    ChatWidget* console = createConsoleWidget();
    connect ( console, SIGNAL(sendText(QString)), SLOT(writeCheckMoves(QString)));
    setConsole ( console );
    ToolWidgetData data;
    data.widget = console;
    data.title = i18n("Console for %1 (%2)", attribute("program").toString(), colorName ( color() ) );
    data.name = QLatin1String("console") + attribute("program").toString() + QLatin1Char( color() == White ? 'W' : 'B' );
    data.type = ConsoleToolWidget;
    data.owner = color();
    return QList< Protocol::ToolWidgetData >() << data;
}

bool XBoardProtocol::parseStub(const QString& line)
{
    parseLine(line);
    return true;
}

void XBoardProtocol::parseLine(const QString& line)
{
        if ( line.isEmpty() )
        {
            return;
        }
        bool display = true;
        ChatWidget::MessageType type = ChatWidget::GeneralMessage;
        if ( line.contains ( QLatin1String ( "Illegal move" ) ) )
        {
            type = ChatWidget::ErrorMessage;
            emit illegalMove();
        }
        else if ( line.contains ( QLatin1String ( "..." ) ) || line.contains(QLatin1String("move")) )
        {
            type = ChatWidget::MoveMessage;
            const QRegExp position(QLatin1String("[a-h][1-8]"));
            QString moveString = line.split ( QLatin1Char ( ' ' ) ).last();
            if ( moveString == lastMoveString )
            {
                return;
            }
            lastMoveString = moveString;
            Move m;
            if ( position.indexIn(line) > -1 )
            {
                m.setString(moveString);
            }
            else if ( moveString.contains(QLatin1String("O-O-O"))
                    || moveString.contains(QLatin1String("o-o-o"))
                    || moveString.contains(QLatin1String("0-0-0")) )
            {
                m = Move::castling(Move::QueenSide, Manager::self()->activePlayer());
            }
            else if ( moveString.contains(QLatin1String("O-O"))
                    || moveString.contains(QLatin1String("o-o"))
                    || moveString.contains(QLatin1String("0-0")) )
            {
                m = Move::castling(Move::KingSide, Manager::self()->activePlayer());
            }
            else
            {
                type = ChatWidget::GeneralMessage;
            }
            if ( m.isValid() )
            {
                kDebug() << "Move by" << attribute("program").toString() << ":" << moveString << "=>" << m;
                emit pieceMoved ( m );
                emit undoPossible ( true );
            }
        }
        else if ( line.contains ( QLatin1String ( "wins" ) ) )
        {
            type = ChatWidget::StatusMessage;
            Color winner;
            if ( line.split ( QLatin1Char ( ' ' ) ).last().contains ( QLatin1String ( "white" ) ) )
            {
                winner = White;
            }
            else
            {
                winner = Black;
            }
            emit gameOver ( winner );
            return;
        }
        else if ( line.contains ( QLatin1String("offer") ) && line.contains ( QLatin1String("draw") ) )
        {
            display = false;
            Offer o;
            o.action = ActionDraw;
            o.id = nextId();
            o.player = color();
            Manager::self()->sendOffer(o);
        }
        else if ( line.startsWith ( QLatin1String("1-0") ) )
        {
            emit gameOver ( White );
        }
        else if ( line.startsWith ( QLatin1String("0-1") ) )
        {
            emit gameOver ( NoColor );
        }
        else if ( line.startsWith ( QLatin1String("1/2-1/2") ) )
        {
            emit gameOver ( Black );
        }
        if ( display )
        {
            writeToConsole ( line, type );
        }
    }

void XBoardProtocol::readError()
{
    kError() << mProcess->readAllStandardError();
}

void XBoardProtocol::acceptOffer(const Offer& offer)
{
    switch ( offer.action )
    {
        case ActionDraw:
            setWinner(NoColor);
            break;
            
        case ActionAdjourn:
            write( QLatin1String("save") + KFileDialog::getSaveFileName() );
            break;
            
        case ActionUndo:
            write ( "force" );
            for ( int i = 0; i < offer.numberOfMoves; ++i )
            {
                write ( "undo" );
            }
            // This function is called before changeActivePlayer, so we must take into accont 
            // the number of moves being undone. 
            if ( ( Manager::self()->activePlayer() == color() ) == ( ( offer.numberOfMoves % 2 ) == 0 ) )
            {
                write ( "go" );
            }
            else
            {
                resumePending = true;
            }
            break;
            
        case ActionPause:
            write ( "force" );
            break;
            
        case ActionResume:
            if ( Manager::self()->activePlayer() == color() )
            {
                write ( "go" );
            }
            else
            {
                resumePending = true;
            }
            break;
            
        default:
            kError() << "XBoard should not send this kind offers";
            break;
    }
}

void XBoardProtocol::declineOffer(const Offer& offer)
{
    // No special action to do here, ignoring an offer is the same as declining. 
    Q_UNUSED(offer);
}

void XBoardProtocol::setWinner(Color winner)
{
    QByteArray result = "result ";
    switch ( winner )
    {
        case White:
            result += "1-0";
            break;
        case Black:
            result += "0-1";
            break;
        case NoColor:
            result += "1/2-1/2";
            break;
    }
    write(QLatin1String(result));
}

void XBoardProtocol::makeOffer(const Offer& offer)
{
    switch ( offer.action )
    {
        case ActionDraw:
            write("draw");
            break;
            
        case ActionAdjourn:
            write( QLatin1String("save") + KFileDialog::getSaveFileName() );
            offer.accept();
            break;
            
        case ActionUndo:
            for ( int i = 0; i < offer.numberOfMoves; ++i )
            {
                write ( "undo" );
            }
            offer.accept();
            break;
            
        case ActionPause:
            write ( "force" );
            offer.accept();
            break;
            
        case ActionResume:
            if ( Manager::self()->activePlayer() == color() )
            {
                write ( "go" );
            }
            else
            {
                resumePending = true;
            }
            offer.accept();
            break;
            
        default:
            break;
    }
}



// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
