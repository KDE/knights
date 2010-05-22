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

using namespace Knights;

int id = qRegisterMetaType<Protocol::ErrorCode>("Protocol::ErrorCode");

QString Protocol::stringFromErrorCode ( Protocol::ErrorCode code )
{
    switch ( code )
    {
        case NoError:
            return i18n ( "No Error" );

        case UserCancelled:
            return i18n ( "User Cancelled" );

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

void Protocol::setPlayerColor ( Piece::Color color )
{
    m_color = color;
}

Piece::Color Protocol::playerColor() const
{
    return m_color;
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

void Protocol::pauseGame()
{

}

void Protocol::resumeGame()
{

}

void Protocol::undoLastMove()
{

}

Move::List Protocol::moveHistory()
{
    return Move::List();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
