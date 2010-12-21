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

#include "protocol.h"
#include "core/move.h"

#include <KLocale>

#include <QtCore/QMetaType>
#include <QStack>

namespace Knights
{

    int id = qRegisterMetaType<Protocol::ErrorCode> ( "Protocol::ErrorCode" );

    class ProtocolPrivate
    {
        public:
            QVariantMap attributes;
            QList<Move> moveHistory;
            QStack<Move> moveUndoStack;
    };

    Protocol::Protocol ( QObject* parent ) : QObject ( parent ), d_ptr ( new ProtocolPrivate )
    {

    }

    Protocol::~Protocol()
    {

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

    void Protocol::addMoveToHistory ( const Knights::Move& move )
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
        return m.reverse();
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

    QWidgetList Protocol::toolWidgets()
    {
        return QWidgetList();
    }

    void Protocol::pauseGame()
    {

    }

    void Protocol::resumeGame()
    {

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


}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
