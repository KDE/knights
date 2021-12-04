/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "piece.h"

#include <KLocalizedString>

namespace Knights {

QString Piece::spriteKey ( PieceType type, Color color ) {
	QString id;
	switch ( color ) {
	case White:
		id.append ( QLatin1String ( "White" ) );
		break;
	case Black:
		id.append ( QLatin1String ( "Black" ) );
		break;
	default:
		break;
	}
	switch ( type ) {
	case Pawn:
		id.append ( QLatin1String ( "Pawn" ) );
		break;
	case Rook:
		id.append ( QLatin1String ( "Rook" ) );
		break;
	case Knight:
		id.append ( QLatin1String ( "Knight" ) );
		break;
	case Bishop:
		id.append ( QLatin1String ( "Bishop" ) );
		break;
	case Queen:
		id.append ( QLatin1String ( "Queen" ) );
		break;
	case King:
		id.append ( QLatin1String ( "King" ) );
		break;
	default:
		break;
	}
	return id;
}

QChar Piece::charFromType ( PieceType t ) {
	switch ( t ) {
	case Pawn:
		return QLatin1Char ( 'P' );
	case Queen:
		return QLatin1Char ( 'Q' );
	case King:
		return QLatin1Char ( 'K' );
	case Bishop:
		return QLatin1Char ( 'B' );
	case Knight:
		return QLatin1Char ( 'N' );
	case Rook:
		return QLatin1Char ( 'R' );
	default:
		break;
	}
	return QLatin1Char ( 'E' );
}

PieceType Piece::typeFromChar ( QChar typeChar ) {
	PieceType pType = Queen;
	if ( typeChar == QLatin1Char ( 'N' ) || typeChar == QLatin1Char ( 'n' ) )
		pType = Knight;
	else if ( typeChar == QLatin1Char ( 'R' ) || typeChar == QLatin1Char ( 'r' ) )
		pType = Rook;
	else if ( typeChar == QLatin1Char ( 'B' ) || typeChar == QLatin1Char ( 'b' ) )
		pType = Bishop;
	else if ( typeChar == QLatin1Char ( 'P' ) || typeChar == QLatin1Char ( 'p' ) )
		pType = Pawn;
	else if ( typeChar == QLatin1Char ( 'K' ) || typeChar == QLatin1Char ( 'k' ) )
		pType = King;
	return pType;
}

Piece::Piece ( KGameRenderer* renderer, PieceType type, Color color, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parent ) :
	Item ( renderer, spriteKey ( type, color ), scene, boardPos, parent ) {
	m_color = color;
	m_type = type;
}

Piece::~Piece() = default;

Color oppositeColor ( Color color ) {
	switch ( color ) {
	case Black:
		return White;
	case White:
		return Black;
	default:
		return color;
	}
}

QString colorName ( Color color ) {
	switch ( color ) {
	case White:
		return i18n ( "White" );
	case Black:
		return i18n ( "Black" );
	default:
		return QString();
	}
}

QString pieceTypeName ( PieceType type ) {
	switch ( type ) {
	case Pawn:
		return i18n ( "Pawn" );
	case Rook:
		return i18n ( "Rook" );
	case Knight:
		return i18n ( "Knight" );
	case Bishop:
		return i18n ( "Bishop" );
	case Queen:
		return i18n ( "Queen" );
	case King:
		return i18n ( "King" );
	default:
		return QString();
	}
}

Color Piece::color() {
	return m_color;
}

PieceType Piece::pieceType() {
	return m_type;
}

void Piece::setPieceType ( PieceType type ) {
	m_type = type;
	updateSpriteKey();
}

void Piece::updateSpriteKey() {
	setSpriteKey ( spriteKey ( m_type, m_color ) );
	update();
}

}
