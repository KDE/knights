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

#ifndef KCHESS_RULES_H
#define KCHESS_RULES_H

#include "core/piece.h"

#include "board.h"
#include <KDebug>

template<class Key, class T > class QMap;
namespace Knights
{
    class Pos;
    class Move;

    class Rules
    {
        public:
            enum Direction
            {
                None = 0,
                N = 0x001,
                S = 0x002,
                W = 0x004,
                E = 0x008,
                NW = 0x010,
                NE = 0x020,
                SW = 0x040,
                SE = 0x080,
                LineDirections = N | S | W | E,
                DiagDirections = NW | NE | SW | SE,
                AllDirections = LineDirections | DiagDirections
            };

            typedef QFlags<Direction> Directions;

            virtual ~Rules()
            {
            }

            virtual void setGrid ( Grid* grid )
            {
                kDebug() << "Setting Grid";
                m_grid = grid;
            }

            virtual QList<Move> legalMoves ( const Pos& pos ) = 0;
            /**
              * @return The positions and types of the staring pieces of the color @a color
              */
            virtual PieceDataMap startingPieces () = 0;

            /**
              * Used to check whether any player has a winning position.
              * @return the color of the winner, or NoColor if no one has won yet
              * @note if the game is in a stalemate, NoColor is returned
              * @sa hasLegalMoves()
              */
            virtual Color winner() = 0;

            /**
              * Check if the player has no legal moves
              * Used for determining stalemates
              */
            virtual bool hasLegalMoves ( Color color ) = 0;

            /**
              * This function is more of a guideline for the board to determine whether a piece should be freely dragable.
              * It is currently not used anywhere in the game.
              */
            virtual Directions legalDirections ( PieceType type ) = 0;

            /**
             * Checks if a piece on @a attackingPos is attacking the opponent's king
             * @param pos the position to check
             * @return true if a piece is attacking the king, false otherwise
             */
            virtual bool isAttacking ( const Pos& attackingPos ) = 0;

            /**
              * Adds appropriate flags to the move.
              * Useful when processing moves from a computer engine, as they only specify the start and end pos,
              * and no other information
              * @param move a reference to the move with either @a from and @a to or its @a string already set.
              * @param color the color if the player who made this move
              */
            virtual void checkSpecialFlags ( Move* move, Color color ) = 0;

            /**
              * Called when a move has been made, either by the player or a computer opponent
              * Useful to update any game state the rules engine has saved
              * @param move The move which was made, with all information about it.
              */
            virtual void moveMade ( const Move& move ) = 0;

        protected:
            Grid *m_grid;
    };

}

#endif // KCHESS_RULES_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
