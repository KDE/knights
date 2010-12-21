/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "localprotocol.h"

using namespace Knights;

void LocalProtocol::init ( const QVariantMap& options )
{
    Q_UNUSED(options)
    emit initSuccesful();
}

void LocalProtocol::startGame()
{

}

void LocalProtocol::move ( const Knights::Move& m )
{
    addMoveToHistory(m);
}

LocalProtocol::LocalProtocol ( QObject* parent ) : Protocol ( parent )
{
    setPlayerColors( White | Black );
}

LocalProtocol::~LocalProtocol()
{

}

Knights::Protocol::Features LocalProtocol::supportedFeatures()
{
    return Pause | Undo | TimeLimit;
}

void LocalProtocol::pauseGame()
{
}

void LocalProtocol::resumeGame()
{
}

void LocalProtocol::undoLastMove()
{
    emit pieceMoved(nextUndoMove());
}

void LocalProtocol::redoLastMove()
{
    emit pieceMoved(nextRedoMove());
}

