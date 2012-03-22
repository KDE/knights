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

#include "proto/xboardprotocol.h"
#include "proto/chatwidget.h"
#include "gamemanager.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>
#include <KFileDialog>
#include <settings.h>

using namespace Knights;

XBoardProtocol::XBoardProtocol ( QObject* parent ) : ComputerProtocol ( parent )
, m_moves(0)
, m_increment(0)
, m_baseTime(0)
, m_timeLimit(0)
{

}

Protocol::Features XBoardProtocol::supportedFeatures()
{
    // FIXME: This shouldn't be hardcoded. For instance, a chess engine which
    // supports the XBoard protocol may or may not support the pause action; we
    // should find this out using the 'protover' command which will reply with
    // the 'feature' command. See the XBoard protocol specification:
    // http://home.hccnet.nl/h.g.muller/engine-intf.html

    //return GameOver | Pause | Draw | Adjourn | Resign | Undo | SetDifficulty | AdjustDifficulty;
    return GameOver | Pause | Draw | Resign | Undo | SetDifficulty | AdjustDifficulty;
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
    startProgram();
    write("xboard");
    initComplete();
}

QList< Protocol::ToolWidgetData > XBoardProtocol::toolWidgets()
{
    return ComputerProtocol::toolWidgets();
}

bool XBoardProtocol::parseStub(const QString& line)
{
    parseLine(line);
    return true;
}

bool XBoardProtocol::parseLine(const QString& line)
{
        if ( line.isEmpty() )
        {
            return true;
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
                return true;
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
            return true;
        }
        else if ( line.contains ( QLatin1String("offer") ) && line.contains ( QLatin1String("draw") ) )
        {
            display = false;
            Offer o;
            o.action = ActionDraw;
            o.id = 0;
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
        return true;
    }

void XBoardProtocol::readError()
{
    kError() << mProcess->readAllStandardError();
}

void XBoardProtocol::acceptOffer(const Offer& offer)
{
    kDebug() << "Accepting offer" << offer.text;
    switch ( offer.action )
    {
        case ActionDraw:
            setWinner(NoColor);
            break;
            
        case ActionAdjourn:
            write( QLatin1String("save ") + KFileDialog::getSaveFileName() );
            break;
            
        case ActionUndo:
            for ( int i = 0; i < offer.numberOfMoves/2; ++i )
            {
                write ( "remove" );
            }
            if (offer.numberOfMoves % 2)
            {
                write ( "force" );
                write ( "undo" );
                
                if ( Manager::self()->activePlayer() != color() )
                {
                    write ( "go" );
                }
                else
                {
                    resumePending = true;
                }
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
            write( QLatin1String("save ") + KFileDialog::getSaveFileName() );
            offer.accept();
            break;
            
        case ActionUndo:
            for ( int i = 0; i < offer.numberOfMoves/2; ++i )
            {
                write ( "remove" );
            }
            if (offer.numberOfMoves % 2)
            {
                write ( "force" );
                write ( "undo" );
                
                if ( Manager::self()->activePlayer() != color() )
                {
                    write ( "go" );
                }
                else
                {
                    resumePending = true;
                }
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

void XBoardProtocol::setDifficulty(int depth, int memory)
{
    // Gnuchess only supports 'depth', while Crafty (and the XBoard protocol) wants 'sd'. 
    // So we give both. 
    write ( QLatin1String("depth ") + QString::number ( depth ) );
    write ( QLatin1String("sd ") + QString::number ( depth ) );
    write ( QLatin1String("memory ") + QString::number ( memory ) );
}



// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
