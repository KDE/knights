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
#include <QtCore/QSharedData>

namespace Knights
{
    class MovePrivate : public QSharedData
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
            : from ( Pos() ),
            to ( Pos() ),
            promotedType ( NoType ),
            flags ( Move::None )
    {
    }

    MovePrivate::~MovePrivate()
    {

    }

    Move Move::castling ( Move::CastlingSide side, Color color )
    {
        Move m;
        const int rank = ( color == White ) ? 1 : 8;
        const int rookFile = ( side == QueenSide ) ? 1 : 8;
        const int kingDestinationFile = ( side == QueenSide ) ? 3 : 7;
        const int rookDestinationFile = ( side == QueenSide ) ? 4 : 6;

        m.setFrom ( 5, rank );
        m.setTo ( kingDestinationFile, rank );

        m.setFlag ( Castle, true );
        Move rookMove;
        rookMove.setFrom ( rookFile, rank );
        rookMove.setTo ( rookDestinationFile, rank );
        m.setAdditionalMoves ( QList<Move>() << rookMove );

        return m;
    }

    Move::Move()
            : d ( new MovePrivate )
    {

    }

    Move::Move ( Pos from, Pos to, Move::Flags flags )
            : d ( new MovePrivate )
    {
        setFrom ( from );
        setTo ( to );
        setFlags ( flags );
    }

    Move::Move ( QString string )
            : d ( new MovePrivate )
    {
        setString ( string );
    }

    Move::Move ( const Knights::Move& other )
            : d ( other.d )
    {

    }

    Move::~Move()
    {

    }

    void Move::operator= ( const Knights::Move& other )
    {
        d = other.d;
    }

    Move::Flags Move::flags() const
    {
        return d->flags;
    }

    void Move::setFlag ( Move::MoveFlag flag, bool value )
    {
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
        d->flags = flags;
    }

    void Move::setString ( QString string )
    {
        d->flags = None;
        d->extraCaptures.clear();
        d->extraMoves.clear();

        if ( string.contains ( QLatin1Char ( 'x' ) ) )
        {
            setFlag ( Take, true );
            string.remove ( QLatin1Char ( 'x' ) );
        }
        setFrom ( string.left ( 2 ) );
        setTo ( string.mid ( 2, 2 ) );
        if ( string.size() > 5 )
        {
            setFlag ( Promote, true );
            setPromotedType ( Piece::typeFromChar ( string[5] ) );
        }
    }

    QString Move::string ( bool includeX ) const
    {
        QString str = from().string();
        if ( ( flags() & Take ) && includeX )
        {
            str += QLatin1Char ( 'x' );
        }
        str += to().string();
        if ( flags() & Promote )
        {
            str += QLatin1Char ( '=' );
            str += Piece::charFromType ( promotedType() );
        }
        return str;
    }


    Pos Move::from() const
    {
        return d->from;
    }

    Pos Move::to() const
    {
        return d->to;
    }


    void Move::setFrom ( const Knights::Pos& value )
    {
        d->from = value;
    }

    void Move::setFrom ( int first, int second )
    {
        setFrom ( Pos ( first, second ) );
    }


    void Move::setTo ( const Knights::Pos& value )
    {
        d->to = value;
    }


    void Move::setTo ( int first, int second )
    {
        d->to = Pos ( first, second );
    }

    const QList< Pos >& Move::additionalCaptures() const
    {
        return d->extraCaptures;
    }

    void Move::setAdditionalCaptures ( const QList< Pos >& list )
    {
        d->extraCaptures = list;
    }

    const QList< Move >& Move::additionalMoves() const
    {
        return d->extraMoves;
    }

    void Move::setAdditionalMoves ( const QList< Move >& list )
    {
        d->extraMoves = list;
    }

    PieceType Move::promotedType() const
    {
        return d->promotedType;
    }

    void Move::setPromotedType ( PieceType type )
    {
        d->promotedType = type;
    }

    bool Move::operator== ( Move other ) const
    {
        return ( d->from == other.from() && d->to == other.to() );
    }




}


// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
