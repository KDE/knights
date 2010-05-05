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

#ifndef KNIGHTS_PROTOCOL_H
#define KNIGHTS_PROTOCOL_H

#include <core/move.h>
#include <core/piece.h>

#include <QtCore/QObject>

namespace Knights
{

class Protocol : public QObject
{
    Q_OBJECT
    Q_ENUMS(Feature)
    Q_FLAGS(Features)

    public:

    enum Feature
    {
      NoFeatures = 0,
      TimeLimit = 1,
      Pause = 2,
      History = 4,
      Undo = 8
    };

    typedef QFlags<Feature> Features;

    
    Protocol(QObject* parent = 0) : QObject(parent) {};
    virtual ~Protocol() {};

    // Needed functions
    
    virtual bool init(QVariantMap options) = 0;
    virtual void setPlayerColor(Piece::Color color) = 0;

public Q_SLOTS:
    virtual void move(Move m) = 0;
    virtual void startGame() = 0;


    // Optional features
    public:

      virtual Features supportedFeatures();
      virtual Move::List moveHistory();
      int timeRemaining();
      
    public Q_SLOTS:
      
    virtual void pauseGame();
    virtual void resumeGame();
    virtual void undoLastMove();
    virtual void setOpponentTimeLimit(int seconds);
    virtual void setPlayerTimeLimit(int seconds);

Q_SIGNALS:
    void pieceMoved(Move m);
    void illegalMove();
    void gameOver(Piece::Color winner);
};

}

#endif // KNIGHTS_PROTOCOL_H
