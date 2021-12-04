/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KCHESS_RULES_H
#define KCHESS_RULES_H

#include "board.h"
#include "core/piece.h"
#include "knightsdebug.h"

template<class Key, class T > class QMap;
namespace Knights {
class Pos;
class Move;

class Rules {
public:
	enum Direction {
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

	virtual ~Rules() = default;

	virtual void setGrid ( Grid* grid ) {
		qCDebug(LOG_KNIGHTS) << "Setting Grid";
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
