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

#ifndef KCHESS_CHESSRULES_H
#define KCHESS_CHESSRULES_H

#include <rules.h>

#include <QtCore/QMap>
#include <QtCore/QStack>

namespace Knights
{
    class Move;
    class Pos;

    class ChessRules : public Knights::Rules
    {
        public:

            ChessRules();

            virtual Piece::Color winner();
            virtual bool hasLegalMoves ( Piece::Color color );
            virtual QMap< Pos, Piece::PieceType > startingPieces ( Piece::Color color );
            virtual QList<Move> legalMoves ( const Pos& pos );
            virtual void moveMade ( const Move& move );
            virtual Directions legalDirections ( Piece::PieceType type );
            virtual ~ChessRules();

            bool isAttacked ( const Pos& pos, Piece::Color color, Grid * grid = 0 );

            virtual void checkSpecialFlags ( Move* move );

        private:

            struct MoveData
            {
                Move m;
                Piece::PieceType pieceType;
                Piece::Color color;
            };

            QMap<Direction, Pos> directions;
            QMap<Direction, Pos> lineDirs;
            QMap<Direction, Pos> diagDirs;
            QList<Pos> knightDirs;
            QStack<MoveData> moveHistory;

        private:
            QList<Move> movesInDirection ( const Pos& dir, const Pos& pos, int length = 8, bool attackYours = false, Grid* grid = 0 );
            QList<Move> pawnMoves ( const Pos& pos );
            QList<Move> castlingMoves ( const Pos& pos );
            int length ( const Move* move );
            bool isPathClear ( const Pos& from, const Pos& to );
            QList<Move> legalAttackMoves ( const Pos& pos, Grid* grid = 0 );
            bool isKingAttacked ( Piece::Color color , Grid* grid = 0 );
            QMap<Piece::Color, bool> kingMoved;
            QMap<Piece::Color, bool> kingRookMoved;
            QMap<Piece::Color, bool> queenRookMoved;
            QMap<Piece::Color, Pos> queenRookStartPos;
            QMap<Piece::Color, Pos> kingRookStartPos;
            QList<Move> m_enPassantMoves;
            QMap<Piece::Color, Pos> kingPos;
    };
}

#endif // KCHESS_CHESSRULES_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
