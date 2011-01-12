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

#include "proto/protocol.h"
#include "proto/chatwidget.h"
#include "core/move.h"

#include <KLocale>

#include <QtCore/QStack>
#include <QtCore/QTime>
#include <KDebug>

namespace Knights
{
    const int TimerInterval = 100; // miliseconds
    int id = qRegisterMetaType<Protocol::ErrorCode> ( "Protocol::ErrorCode" );

    class ProtocolPrivate
    {
        public:

            ProtocolPrivate();
            
            QVariantMap attributes;
            QList<Move> moveHistory;
            QStack<Move> moveUndoStack;
            Protocol::TimeControl whiteTimeControl;
            Protocol::TimeControl blackTimeControl;
            Color activePlayer;

            int timer;
            bool running;
    };

    ProtocolPrivate::ProtocolPrivate()
    : timer(0)
    , running(false)
    {

    }


    Protocol::Protocol ( QObject* parent ) : QObject ( parent ), d_ptr ( new ProtocolPrivate )
    {
        setActivePlayer ( White );
    }

    Protocol::~Protocol()
    {

    }

void Protocol::startTime()
{
    Q_D(Protocol);
    if ( !d->running )
    {
        d->timer = startTimer ( TimerInterval );
        d->running = true;
    }
}

void Protocol::stopTime()
{
    Q_D(Protocol);
    if ( d->running )
    {
        killTimer(d->timer);
        d->running = false;
    }
}

void Protocol::setCurrentTime(Color color, const QTime& time)
{
    Q_D(Protocol);
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

    void Protocol::timerEvent(QTimerEvent* )
    {
        Q_D(Protocol);
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

    QString Protocol::stringFromErrorCode ( Protocol::ErrorCode code )
    {
        switch ( code )
        {
            case NoError:
                return i18n ( "No Error" );

            case UserCancelled:
                return i18n ( "User Canceled" );

            case NetworkError:
                return i18n ( "Network Error" );

            case UnknownError:
                return i18n ( "Unknown Error" );

            case InstallationError:
                return i18n ( "Program Error" );

            default:
                return QString();
        }
    }

    void Protocol::setPlayerColor ( Color color )
    {
        setPlayerColors( color );
    }

    Color Protocol::playerColor() const
    {
        Colors colors = playerColors();
        if ( colors == White )
        {
            return White;
        }
        if ( colors == Black )
        {
            return Black;
        }
        return NoColor;
    }

    void Protocol::setPlayerColors( Colors colors )
    {
        setAttribute ( "PlayerColors", QVariant::fromValue<Colors>( colors ) );
    }

    Colors Protocol::playerColors() const
    {
        return attribute("PlayerColors").value<Colors>();
    }

    void Protocol::setOpponentName ( const QString& name )
    {
        setAttribute ( QLatin1String ( "OpponentName" ), name );
    }

    QString Protocol::opponentName() const
    {
        return attribute ( QLatin1String ( "OpponentName" ) ).toString();
    }

    void Protocol::setPlayerName ( const QString& name )
    {
        setAttribute ( QLatin1String ( "PlayerName" ), name );
    }

    QString Protocol::playerName() const
    {
        return attribute ( QLatin1String ( "PlayerName" ) ).toString();
    }

    void Protocol::setAttribute ( const QString& attribute, QVariant value )
    {
        Q_D ( Protocol );
        d->attributes.insert ( attribute,  value );
    }

    void Protocol::setAttribute ( const char* attribute, QVariant value )
    {
        setAttribute( QLatin1String ( attribute ), value );
    }

    void Protocol::setAttributes ( QVariantMap attributes )
    {
        Q_D ( Protocol );
        d->attributes.unite ( attributes );
    }

    QVariant Protocol::attribute ( const QString& attribute ) const
    {
        Q_D ( const Protocol );
        return d->attributes.value ( attribute );
    }

    QVariant Protocol::attribute ( const char* attribute ) const
    {
        return this->attribute ( QLatin1String ( attribute ) );
    }

    void Protocol::addMoveToHistory ( const Move& move )
    {
        Q_D ( Protocol );
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

    Move Protocol::nextUndoMove()
    {
        Q_D ( Protocol );
        Move m = d->moveHistory.takeLast();
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

    Move Protocol::nextRedoMove()
    {
        Q_D ( Protocol );
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

    Protocol::Features Protocol::supportedFeatures()
    {
        return NoFeatures;
    }

    void Protocol::setOpponentTimeLimit ( int seconds )
    {
        Q_UNUSED ( seconds )
    }

    void Protocol::setPlayerTimeLimit ( int seconds )
    {
        Q_UNUSED ( seconds )
    }

    int Protocol::timeRemaining()
    {
        return -1;
    }

    QList< Protocol::ToolWidgetData > Protocol::toolWidgets()
    {
        return QList< Protocol::ToolWidgetData >();
    }

    void Protocol::pauseGame()
    {
        startTime();
    }

    void Protocol::resumeGame()
    {
        stopTime();
    }

    void Protocol::undoLastMove()
    {

    }

    void Protocol::redoLastMove()
    {

    }

    Move::List Protocol::moveHistory()
    {
        return Move::List();
    }

    void Protocol::adjourn()
    {

    }
    void Protocol::proposeDraw()
    {

    }
    void Protocol::resign()
    {

    }
    
void Protocol::setWinner(Color winner)
{
    Q_UNUSED(winner);
}

void Protocol::setTimeControl(Color color, int moves, int baseTime, int increment)
{
    setTimeControl(color, moves, QTime().addSecs(60 * baseTime), increment);
}

void Protocol::setTimeControl(Color color, int moves, const QTime& baseTime, int increment)
{
    TimeControl c;
    c.baseTime = baseTime;
    c.moves = moves;
    c.increment = increment;
    setTimeControl ( color, c );
}

void Protocol::setTimeControl(Color color, const TimeControl& control)
{
    Q_D(Protocol);
    TimeControl c = control;
    c.currentTime = c.baseTime;
    if ( color == NoColor )
    {
        setTimeControl ( White, c );
        setTimeControl ( Black, c );
        return;
    }
    if ( color == White )
    {
        d->whiteTimeControl = c;
    }
    else
    {
        d->blackTimeControl = c;
    }
    emit timeLimitChanged ( color, c.baseTime );
}

Protocol::TimeControl Protocol::timeControl(Color color) const
{
    Q_D(const Protocol);
    return (color == White) ? d->whiteTimeControl : d->blackTimeControl;
}

QTime Protocol::timeLimit(Color color)
{
    Q_D(Protocol);
    return ( color == White ) ? d->whiteTimeControl.baseTime : d->blackTimeControl.baseTime;
}

ChatWidget* Protocol::createChatWidget()
{
    return new ChatWidget;
}

ChatWidget* Protocol::createConsoleWidget()
{
    ChatWidget* console = new ChatWidget;
    console->setConsoleMode(true);
    return console;
}

void Protocol::setActivePlayer(Color player)
{
    Q_D(Protocol);
    d->activePlayer = player;
}

void Protocol::changeActivePlayer()
{
    setActivePlayer ( oppositeColor ( activePlayer() ) );
}

Color Protocol::activePlayer() const
{
    Q_D(const Protocol);
    return d->activePlayer;
}

}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
