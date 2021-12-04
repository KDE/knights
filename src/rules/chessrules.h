/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KCHESS_CHESSRULES_H
#define KCHESS_CHESSRULES_H

#include <rules/rules.h>
#include <core/move.h>

#include <QMap>
#include <QStack>

namespace Knights {
class Move;
class Pos;

class ChessRules : public Rules {

public:
	ChessRules();
	~ChessRules() override;

	Color winner() override;
	bool hasLegalMoves ( Color color ) override;
	PieceDataMap startingPieces () override;
	QList<Move> legalMoves ( const Pos& pos ) override;
	void moveMade ( const Move& m ) override;
	Directions legalDirections ( PieceType type ) override;

	bool isAttacked ( const Pos& pos, Color color, Grid * grid = nullptr );
	bool isAttacking ( const Pos& attackingPos ) override;

	void checkSpecialFlags ( Move* move, Color color ) override;
	virtual void changeNotation ( Knights::Move* move, Move::Notation notation, Color color );

private:
	struct MoveData {
		Move m;
		PieceType pieceType;
		Color color;
	};

	QMap<Direction, Pos> directions;
	QMap<Direction, Pos> lineDirs;
	QMap<Direction, Pos> diagDirs;
	QList<Pos> knightDirs;
	QStack<MoveData> moveHistory;

	QList<Move> movesInDirection ( const Pos& dir, const Pos& pos, int length = 8, bool attackYours = false, Grid* grid = nullptr );
	QList<Move> pawnMoves ( const Pos& pos );
	QList<Move> castlingMoves ( const Pos& pos );
	int length ( const Move& move );
	bool isPathClearForCastling ( const Pos& kingPos, const Pos& rookPos );
	QList<Move> legalAttackMoves ( const Pos& pos, Grid* grid = nullptr );
	bool isKingAttacked ( Color color, Grid* grid = nullptr );

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
