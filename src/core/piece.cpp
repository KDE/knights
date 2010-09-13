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

#include "piece.h"

#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtSvg/QSvgRenderer>

namespace Knights {
    
QString Piece::spriteKey(PieceType type, Color color)
{
QString id;
    switch ( color )
    {
        case White:
            id.append ( "White" );
            break;
        case Black:
            id.append ( "Black" );
            break;
        default:
            break;
    }
    switch ( type )
    {
        case Pawn:
            id.append ( "Pawn" );
            break;
        case Rook:
            id.append ( "Rook" );
            break;
        case Knight:
            id.append ( "Knight" );
            break;
        case Bishop:
            id.append ( "Bishop" );
            break;
        case Queen:
            id.append ( "Queen" );
            break;
        case King:
            id.append ( "King" );
            break;
        default:
            break;
    }
    return id;
}

QChar Piece::charFromType(PieceType t)
{
    switch ( t )
    {
        case Pawn:
            return 'P';
        case Queen:
            return 'Q';
        case King:
            return 'K';
        case Bishop:
            return 'B';
        case Knight:
            return 'N';
        case Rook:
            return 'R';
        default:
            break;
    }
    return 'E';
}

PieceType Piece::typeFromChar(QChar typeChar)
{
                    PieceType pType = Queen;
                    if ( typeChar == 'N' || typeChar == 'n' )
                    {
                        pType = Knight;
                    } 
                    else if ( typeChar == 'R' || typeChar == 'r' )
                    {
                        pType = Rook;
                    }
                    else if ( typeChar == 'K' || typeChar == 'k' )
                    {
                        pType = Bishop;
                    }
                    else if ( typeChar == 'P' || typeChar == 'p' )
                    {
                        pType = Pawn;
                    }
                    else if ( typeChar == 'K' || typeChar == 'k' )
                    {
                        pType = King;
                    }
                    return pType;
}


Piece::Piece(Renderer* renderer, PieceType type, Color color, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parent): 
Item(renderer, spriteKey(type, color), scene, boardPos, parent)
{
    m_color = color;
    m_type = type;
}


Piece::~Piece()
{

}

Color oppositeColor ( Color color )
{
    switch ( color )
    {
    case Black:
        return White;
    case White:
        return Black;
    default:
        return color;
    }
}

Color Piece::color()
{
    return m_color;
}

PieceType Piece::pieceType()
{
    return m_type;
}

void Piece::setPieceType(PieceType type)
{
    m_type = type;
    updateSpriteKey();
}

void Piece::updateSpriteKey()
{
    setSpriteKey(spriteKey(m_type, m_color));
    update();
}



}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
