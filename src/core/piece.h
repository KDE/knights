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

#ifndef KNIGHTS_PIECE_H
#define KNIGHTS_PIECE_H

#include "pos.h"
#include "item.h"

namespace Knights
{
    enum PieceType
    {
        NoType = 0,
        King,
        Queen,
        Bishop,
        Knight,
        Rook,
        Pawn,
        PieceTypeCount
    };
    enum Color
    {
        NoColor = 0x00,
        White = 0x01,
        Black = 0x02
    };

    Q_DECLARE_FLAGS(Colors, Color)
    Q_DECLARE_OPERATORS_FOR_FLAGS(Colors)
    Color oppositeColor ( Color color );
    QString colorName ( Color color );
    QString pieceTypeName ( PieceType type );

    class Piece : public Item
    {
            Q_OBJECT
        public:
            Piece ( KGameRenderer* renderer, PieceType type, Color color, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parent = 0 );
            virtual ~Piece();

            PieceType pieceType();
            void setPieceType ( PieceType type );
            Color color();

            static QString spriteKey ( PieceType type, Color color );
            static PieceType typeFromChar ( QChar typeChar );
            static QChar charFromType ( PieceType t );

        private:
            Color m_color;
            PieceType m_type;
            void updateSpriteKey();
    };

    typedef QMap<Pos, Piece*> Grid;
    typedef QPair<Color, PieceType> PieceData;
    typedef QMap<Pos, PieceData> PieceDataMap;
}

Q_DECLARE_METATYPE ( Knights::Color )
Q_DECLARE_METATYPE ( Knights::Colors )
Q_DECLARE_METATYPE ( Knights::PieceType )
Q_DECLARE_METATYPE ( Knights::PieceData )
Q_DECLARE_METATYPE ( Knights::PieceDataMap )

#endif // KNIGHTS_PIECE_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
