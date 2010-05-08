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

#ifndef KNIGHTS_MOVE_H
#define KNIGHTS_MOVE_H

#include "pos.h"

#include <QtCore/QFlags>
#include <QtCore/QPair>
#include <QtCore/QMetaType>

namespace Knights
{
class MovePrivate;
class Move
{
public:

  typedef QList<Move> List;
  
    enum MoveFlag {
        None = 0x00,
        Take = 0x01,
        Promote = 0x02,
        Castle = 0x04,
        EnPassant = 0x08,
        Check = 0x10,
        CheckMate = 0x20,
    };
    Q_DECLARE_FLAGS(Flags, MoveFlag)

    Move ( Pos from, Pos to, Flags flags = None );
    Move();
    Move(const Move& other);
    virtual ~Move();

    Pos from() const;
    Pos to() const;
    void setFrom(Pos value);
    void setFrom(int first, int second);
    void setTo(Pos value);
    void setTo(int first, int second);

    Flags flags() const;
    void setFlag(MoveFlag, bool value);
    void setFlags(Flags);

    const QList<Move>& additionalMoves() const;
    void setAdditionalMoves(const QList<Move>& list);
    const QList<Pos>& additionalCaptures() const;
    void setAdditionalCaptures(const QList<Pos>& list);

    bool operator==(Move other) const;

private:
    Q_DECLARE_PRIVATE(Move)

    MovePrivate* d_ptr;
    Flags m_flags;

};

Q_DECLARE_OPERATORS_FOR_FLAGS(Move::Flags)
}
Q_DECLARE_METATYPE(Knights::Move)
Q_DECLARE_METATYPE(Knights::Move::List)


#endif // KNIGHTS_MOVE_H
