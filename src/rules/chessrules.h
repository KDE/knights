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

#ifndef KCHESS_CHESSRULES_H
#define KCHESS_CHESSRULES_H

#include <rules/rules.h>

#include <QtCore/QMap>
#include <QtCore/QStack>

namespace Knights
{
    class Move;
    class Pos;

    class ChessRules : public Rules
    {
        public:

            ChessRules();

            virtual Color winner();
            virtual bool hasLegalMoves ( Color color );
            virtual PieceDataMap startingPieces ();
            virtual QList<Move> legalMoves ( const Pos& pos );
            virtual void moveMade ( const Move& m );
            virtual Directions legalDirections ( PieceType type );
            virtual ~ChessRules();

            bool isAttacked ( const Pos& pos, Color color, Grid * grid = 0 );
            virtual bool isAttacking ( const Pos& attackingPos );

            virtual void checkSpecialFlags ( Move* move, Color color );
            virtual void changeNotation ( Knights::Move* move, Move::Notation notation, Color color );

        private:

            struct MoveData
            {
                Move m;
                PieceType pieceType;
                Color color;
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
            int length ( const Move& move );
            bool isPathClearForCastling ( const Pos& kingPos, const Pos& rookPos );
            QList<Move> legalAttackMoves ( const Pos& pos, Grid* grid = 0 );
            bool isKingAttacked ( Color color , Grid* grid = 0 );
            
            bool hasKingMoved ( Color color );
            bool hasRookMoved ( Color color, Move::CastlingSide side );
            
            QMap<Color, bool> kingMoved;
            QMap<Color, bool> kingRookMoved;
            QMap<Color, bool> queenRookMoved;
            QMap<Color, Pos> queenRookStartPos;
            QMap<Color, Pos> kingRookStartPos;
            QList<Move> m_enPassantMoves;
            QMap<Color, Pos> kingPos;
    };
}

#endif // KCHESS_CHESSRULES_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
