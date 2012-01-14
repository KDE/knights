/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>

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
#include <QtCore/QSharedDataPointer>
#include "piece.h"

namespace Knights
{
    class MovePrivate;

    class Move
    {
        public:

            typedef QList<Move> List;
            
            enum MoveFlag
            {
                None = 0x000,
                Take = 0x001,
                Promote = 0x002,
                Castle = 0x004,
                EnPassant = 0x008,
                Check = 0x010,
                CheckMate = 0x020,
                Additional = 0x040,
                Forced = 0x080,
                Illegal = 0x100
            };
            enum CastlingSide
            {
                QueenSide,
                KingSide
            };
            enum Notation
            {
                NoNotation, /**< No string has been set */
                Algebraic, /**< It omits the starting file and rank of the piece, unless it is necessary to disambiguate the move. */
                Coordinate, /**< specifies the starting and end position */
                LongAlgebraic /**< specifies almost everything */
            };
            Q_DECLARE_FLAGS ( Flags, MoveFlag )

            static Move castling ( CastlingSide side, Color color );

            Move ( Pos from, Pos to, Flags flags = None );
            Move ( QString string );
            Move();
            Move ( const Move& other );
            virtual ~Move();

            void operator= ( const Move& other );

            Pos from() const;
            Pos to() const;
            QString string ( bool includeX = true ) const;
            Move reverse() const;

            void setFrom ( const Pos& value );
            void setFrom ( int first, int second );
            void setTo ( const Pos& value );
            void setTo ( int first, int second );
            void setString ( QString string );

            bool flag ( Move::MoveFlag flag ) const;
            Flags flags() const;
            void setFlag ( MoveFlag, bool value );
            void setFlags ( Flags );

            const QList<Move>& additionalMoves() const;
            void setAdditionalMoves ( const QList<Move>& list );

            PieceType promotedType() const;
            void setPromotedType ( PieceType type );

            const PieceDataMap& removedPieces() const;
            void setRemovedPieces ( const PieceDataMap& map );
            void addRemovedPiece ( const Pos& pos, const PieceData& data );
            
            const PieceDataMap& addedPieces() const;
            void setAddedPieces ( const PieceDataMap& map );
            void addAddedPiece ( const Pos& pos, const PieceData& data );

            Notation notation() const;

            bool operator== ( Move other ) const;
            void toCoordinateNotation ( Grid grid );
            
            /**
             * Sets the time at which this move was made.
             * It is used to reset the clock after undoing.
             */
            void setTime ( const QTime& time );
            /**
             * Returns the time at which the move was made.
             */
            QTime time();
            
            void setPieceData ( const PieceData& data );
            PieceData pieceData() const;

            bool isValid() const;
            
            void setStringForNotation ( Notation notation, const QString& string );
            QString stringForNotation ( Notation notation ) const;

        private:
            QSharedDataPointer<MovePrivate> d;
    };
    Q_DECLARE_OPERATORS_FOR_FLAGS ( Move::Flags )
}

QDebug operator<< ( QDebug debug, const Knights::Move &move );

Q_DECLARE_METATYPE ( Knights::Move )
Q_DECLARE_METATYPE ( Knights::Move::List )


#endif // KNIGHTS_MOVE_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
