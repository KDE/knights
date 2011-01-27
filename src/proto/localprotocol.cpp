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


#include "localprotocol.h"

#include <KDE/KLocale>
#include "gamemanager.h"

using namespace Knights;

void LocalProtocol::init (  )
{
    initComplete();
}

void LocalProtocol::startGame()
{

}

void LocalProtocol::move ( const Move& m )
{
    ++movesSoFar;
    Manager::self()->addMoveToHistory(m);
    movesSoFar > 1 ? Manager::self()->startTime() : Manager::self()->stopTime();
}

LocalProtocol::LocalProtocol ( QObject* parent ) : Protocol ( parent ),
  movesSoFar(0)
{
}

LocalProtocol::~LocalProtocol()
{

}

bool LocalProtocol::isLocal()
{
  return true;
}

Knights::Protocol::Features LocalProtocol::supportedFeatures()
{
    return Pause | Undo | TimeLimit;
}

void LocalProtocol::undoLastMove()
{
    --movesSoFar;
    emit pieceMoved(Manager::self()->nextUndoMove());
}

void LocalProtocol::redoLastMove()
{
    ++movesSoFar;
    emit pieceMoved(Manager::self()->nextRedoMove());
}

void LocalProtocol::makeOffer(Offer offer)
{
}


