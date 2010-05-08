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

#include "move.h"

#include <QtCore/QPair>

namespace Knights
{

class MovePrivate
{
public:
    MovePrivate();
    ~MovePrivate();

    Pos from;
    Pos to;

    QList<Move> extraMoves;
    QList<Pos> extraCaptures;
};

Move::Flags Move::flags() const
{
    return m_flags;
}

void Move::setFlag(Move::MoveFlag flag, bool value)
{
    if (value)
    {
        m_flags |= flag;
    }
    else
    {
        m_flags &= ~flag;
    }
}

void Move::setFlags(Move::Flags flags)
{
    m_flags = flags;
}

MovePrivate::MovePrivate()
{

}

MovePrivate::~MovePrivate()
{

}


Move::Move()
        : d_ptr(new MovePrivate)
{

}

Move::Move ( Pos from, Pos to, Move::Flags flags )
        : d_ptr(new MovePrivate)
{
    setFrom(from);
    setTo(to);
    setFlags(flags);
}

Move::Move ( const Knights::Move& other )
        : d_ptr(new MovePrivate)
{
    setFrom(other.from());
    setTo(other.to());
    setFlags(other.flags());
    setAdditionalCaptures(other.additionalCaptures());
    setAdditionalMoves(other.additionalMoves());
}


Move::~Move()
{

}


Pos Move::from() const
{
    Q_D(const Move);
    return d->from;
}

Pos Move::to() const
{
    Q_D(const Move);
    return d->to;
}


void Move::setFrom ( Pos value )
{
    Q_D(Move);
    d->from = value;
}

void Move::setFrom ( int first, int second )
{
    setFrom(Pos(first,second));
}


void Move::setTo ( Pos value )
{
    Q_D(Move);
    d->to = value;
}


void Move::setTo ( int first, int second )
{
    Q_D(Move);
    d->to = Pos(first, second);
}

const QList< Pos >& Move::additionalCaptures() const
{
  Q_D(const Move);
  return d->extraCaptures;
}

void Move::setAdditionalCaptures(const QList< Pos >& list)
{
  Q_D(Move);
  d->extraCaptures = list;
}

const QList< Move >& Move::additionalMoves() const
{
  Q_D(const Move);
  return d->extraMoves;
}

void Move::setAdditionalMoves(const QList< Move >& list)
{
  Q_D(Move);
  d->extraMoves = list;
}

bool Move::operator==(Move other) const
{
    Q_D(const Move);
    return (d->from == other.from() && d->to == other.to());
}




}


