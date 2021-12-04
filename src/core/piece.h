/*
 This file is part of Knights, a chess board for KDE SC 4.
 SPDX-FileCopyrightText: 2009-2010 Miha Čančula <miha.cancula@gmail.com>

 SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_PIECE_H
#define KNIGHTS_PIECE_H

#include "pos.h"
#include "item.h"

namespace Knights {
enum PieceType {
	NoType = 0,
	King,
	Queen,
	Bishop,
	Knight,
	Rook,
	Pawn,
	PieceTypeCount
};
enum Color {
	NoColor = 0x00,
	White = 0x01,
	Black = 0x02
};

Q_DECLARE_FLAGS(Colors, Color)
Q_DECLARE_OPERATORS_FOR_FLAGS(Colors)
Color oppositeColor ( Color color );
QString colorName ( Color color );
QString pieceTypeName ( PieceType type );

class Piece : public Item {
	Q_OBJECT
public:
	Piece ( KGameRenderer* renderer, PieceType type, Color color, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parent = nullptr );
	~Piece() override;

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
