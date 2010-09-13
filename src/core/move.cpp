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
    
    PieceType promotedType;
    Move::Flags flags;
};

MovePrivate::MovePrivate()
: from(Pos()),
to(Pos()),
promotedType(NoType),
flags(Move::None)
{
}

MovePrivate::~MovePrivate()
{

}

Move::Flags Move::flags() const
{
    Q_D(const Move);
    return d->flags;
}

void Move::setFlag ( Move::MoveFlag flag, bool value )
{
    Q_D(Move);
    if ( value )
    {
        d->flags |= flag;

    }
    else
    {
        d->flags &= ~flag;
    }
}

void Move::setFlags ( Move::Flags flags )
{
    Q_D(Move);
    d->flags = flags;
}

Move::Move()
        : d_ptr ( new MovePrivate )
{

}

Move::Move ( Pos from, Pos to, Move::Flags flags )
        : d_ptr ( new MovePrivate )
{
    setFrom ( from );
    setTo ( to );
    setFlags ( flags );
}

Move::Move(QString string)
        : d_ptr ( new MovePrivate )
{
    setString(string);
}

Move::Move ( const Knights::Move& other )
        : d_ptr ( new MovePrivate )
{
    setFrom ( other.from() );
    setTo ( other.to() );
    setFlags ( other.flags() );
    setAdditionalCaptures ( other.additionalCaptures() );
    setAdditionalMoves ( other.additionalMoves() );
}


Move::~Move()
{

}

void Move::setString(QString string)
{
    Q_D(Move);
    d->flags = None;
    d->extraCaptures.clear();
    d->extraMoves.clear();
    
    if ( string.contains('x') )
    {
        setFlag(Take, true);
        string.remove('x');
    }
    setFrom(string.left(2));
    setTo(string.mid(2,2));
    if (string.size() > 5)
    {
        setFlag(Promote, true);
        setPromotedType(Piece::typeFromChar(string[5]));
    }
}

QString Move::string() const
{
    QString str = from().string();
    if ( flags() & Take )
    {
        str += 'x';
    }
    str += to().string();
    if ( flags() & Promote )
    {
        str += '=';
        str += Piece::charFromType(promotedType());
    }
    return str;
}


Pos Move::from() const
{
    Q_D ( const Move );
    return d->from;
}

Pos Move::to() const
{
    Q_D ( const Move );
    return d->to;
}


void Move::setFrom ( const Knights::Pos& value )
{
    Q_D ( Move );
    d->from = value;
}

void Move::setFrom ( int first, int second )
{
    setFrom ( Pos ( first, second ) );
}


void Move::setTo ( const Knights::Pos& value )
{
    Q_D ( Move );
    d->to = value;
}


void Move::setTo ( int first, int second )
{
    Q_D ( Move );
    d->to = Pos ( first, second );
}

const QList< Pos >& Move::additionalCaptures() const
{
    Q_D ( const Move );
    return d->extraCaptures;
}

void Move::setAdditionalCaptures ( const QList< Pos >& list )
{
    Q_D ( Move );
    d->extraCaptures = list;
}

const QList< Move >& Move::additionalMoves() const
{
    Q_D ( const Move );
    return d->extraMoves;
}

void Move::setAdditionalMoves ( const QList< Move >& list )
{
    Q_D ( Move );
    d->extraMoves = list;
}

PieceType Move::promotedType() const
{
    Q_D ( const Move );
    return d->promotedType;
}

void Move::setPromotedType(PieceType type)
{
    Q_D ( Move );
    d->promotedType = type;
}



bool Move::operator== ( Move other ) const
{
    Q_D ( const Move );
    return ( d->from == other.from() && d->to == other.to() );
}




}


// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
