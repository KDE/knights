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

#include "rules/chessrules.h"

#include "core/move.h"
#include <KDebug>
#include <QtCore/QMap>

using namespace Knights;

ChessRules::ChessRules()
{
    lineDirs.insert ( E, Pos ( 1, 0 ) );
    lineDirs.insert ( W, Pos ( -1, 0 ) );
    lineDirs.insert ( S, Pos ( 0, -1 ) );
    lineDirs.insert ( N, Pos ( 0, 1 ) );

    diagDirs.insert ( NW, lineDirs[N] + lineDirs[W] );
    diagDirs.insert ( NE, lineDirs[N] + lineDirs[E] );
    diagDirs.insert ( SW, lineDirs[S] + lineDirs[W] );
    diagDirs.insert ( SE, lineDirs[S] + lineDirs[E] );

    directions.unite ( lineDirs ).unite ( diagDirs );

    knightDirs << Pos ( 1, 2 );
    knightDirs << Pos ( -1, 2 );
    knightDirs << Pos ( 1, -2 );
    knightDirs << Pos ( -1, -2 );
    knightDirs << Pos ( 2, 1 );
    knightDirs << Pos ( -2, 1 );
    knightDirs << Pos ( 2, -1 );
    knightDirs << Pos ( -2, -1 );

    queenRookStartPos[White] = Pos ( 1, 1 );
    queenRookMoved[White] = false;
    kingRookStartPos[White] = Pos ( 8, 1 );
    kingRookMoved[Black] = false;
    queenRookStartPos[Black] = Pos ( 1, 8 );
    queenRookMoved[Black] = false;
    kingRookStartPos[Black] = Pos ( 8, 8 );
    kingRookMoved[Black] = false;

    kingMoved[White] = false;
    kingMoved[Black] = false;

    kingPos[White] = Pos ( 5, 1 );
    kingPos[Black] = Pos ( 5, 8 );
}

bool ChessRules::hasLegalMoves ( Color color )
{
    Grid::const_iterator it = m_grid->constBegin();
    Grid::const_iterator end = m_grid->constEnd();
    for ( ; it != end; ++it )
    {
        if ( it.value() && it.value()->color() == color && !legalMoves ( it.key() ).isEmpty() )
        {
            return true;
        }
    }
    return false;
}

Knights::Color ChessRules::winner()
{
    if ( isKingAttacked ( Black ) && !hasLegalMoves ( Black ) )
    {
        return White;
    }
    if ( isKingAttacked ( White ) && !hasLegalMoves ( White ) )
    {
        return Black;
    }
    return NoColor;
}

BoardState ChessRules::startingPieces ( )
{
    BoardState pieces;
    // First, the white pieces
    int baseLine = 0;
    int pawnLine = 0;
    const int kingRow = 5;
    const int queenRow = 4;
    baseLine = 1;
    pawnLine = 2;

    pieces.insert ( Pos ( kingRow, baseLine ), PieceData ( White, King ) );
    pieces.insert ( Pos ( queenRow, baseLine ), PieceData ( White, Queen ) );
    for ( int i = 0; i < 2; ++i )
    {
        pieces.insert ( Pos ( 1 + i*7, baseLine ), PieceData ( White, Rook ) );
        pieces.insert ( Pos ( 2 + i*5, baseLine ), PieceData ( White, Knight ) );
        pieces.insert ( Pos ( 3 + i*3, baseLine ), PieceData ( White, Bishop ) );
    }
    for ( int i = 1; i < 9; ++i )
    {
        pieces.insert ( Pos ( i, pawnLine ), PieceData ( White, Pawn ) );
    }

    // And now the black ones

    baseLine = 8;
    pawnLine = 7;
    pieces.insert ( Pos ( kingRow, baseLine ), PieceData ( Black, King ) );
    pieces.insert ( Pos ( queenRow, baseLine ), PieceData ( Black, Queen ) );
    for ( int i = 0; i < 2; ++i )
    {
        pieces.insert ( Pos ( 1 + i*7, baseLine ), PieceData ( Black, Rook ) );
        pieces.insert ( Pos ( 2 + i*5, baseLine ), PieceData ( Black, Knight ) );
        pieces.insert ( Pos ( 3 + i*3, baseLine ), PieceData ( Black, Bishop ) );
    }
    for ( int i = 1; i < 9; ++i )
    {
        pieces.insert ( Pos ( i, pawnLine ), PieceData ( Black, Pawn ) );
    }
    return pieces;
}

bool ChessRules::isKingAttacked ( Color color, Grid* grid )
{
    return isAttacked ( kingPos[color], color, grid );
}

QList<Move> ChessRules::legalMoves ( const Pos& pos )
{
    QList<Move> moves;
    bool isKingMoving = false;
    Color color = m_grid->value ( pos )->color();
    switch ( m_grid->value ( pos )->pieceType() )
    {
        case King:
            foreach ( const Pos& d, directions )
            {
                foreach ( const Move& m, movesInDirection ( d, pos, 1 ) )
                {
                    moves << m;
                }
            }
            isKingMoving = true;
            moves << castlingMoves ( pos );
            break;
        case Queen:
            foreach ( const Pos& d, directions )
            {
                moves << movesInDirection ( d, pos );
            }
            break;
        case Bishop:
            foreach ( const Pos& d, diagDirs )
            {
                moves << movesInDirection ( d, pos );
            }
            break;
        case Rook:
            foreach ( const Pos& d, lineDirs )
            {
                moves << movesInDirection ( d, pos );
            }
            break;
        case Knight:
            foreach ( const Pos& d, knightDirs )
            {
                if ( !Board::isInBoard ( pos + d ) )
                {
                    continue;
                }
                if ( !m_grid->contains ( pos + d ) )
                {
                    moves << Move ( pos, pos + d );
                }
                else if ( m_grid->value ( pos + d )->color() != color )
                {
                    moves << Move ( pos, pos + d, Move::Take );
                }
            }
            break;
        case Pawn:
            moves << pawnMoves ( pos );
            foreach ( const Move& m, m_enPassantMoves )
            {
                if ( m.from() == pos )
                {
                    moves << m;
                }
            }
            break;
        default:
            break;
    }
    foreach ( const Move& m, moves )
    {
        Grid t_grid = *m_grid;
        t_grid.insert ( m.to(), t_grid.take ( pos ) );
        if ( ( isKingMoving && isAttacked ( m.to(), color, &t_grid ) ) || ( !isKingMoving && isAttacked ( kingPos[color], color, &t_grid ) ) )
        {
            moves.removeAll ( m );
        }
    }
    return moves;
}

QList<Move> ChessRules::legalAttackMoves ( const Knights::Pos& pos, Grid* grid )
{
    if ( !grid )
    {
        grid = m_grid;
    }
    // Defending your pieces is counted here, hence all there 'true's in movesInDirection calls
    QList<Move> moves;
    switch ( grid->value ( pos )->pieceType() )
    {
        case King:
            foreach ( const Pos& d, directions )
            {
                moves << movesInDirection ( d, pos, 1, true, grid );
            }
            break;
        case Queen:
            foreach ( const Pos& d, directions )
            {
                moves << movesInDirection ( d, pos, 8, true, grid );
            }
            break;
        case Bishop:
            foreach ( const Pos& d, diagDirs )
            {
                moves << movesInDirection ( d, pos, 8, true, grid );
            }
            break;
        case Rook:
            foreach ( const Pos& d, lineDirs )
            {
                moves << movesInDirection ( d, pos, 8, true, grid );
            }
            break;
        case Knight:
            foreach ( const Pos& d, knightDirs )
            {
                if ( Board::isInBoard ( pos + d ) )
                {
                    moves << Move ( pos, pos + d );
                }
            }
            break;
        case Pawn:
            if ( grid->value ( pos )->color() == White )
            {
                moves << movesInDirection ( directions[NE], pos, 1, true, grid );
                moves << movesInDirection ( directions[NW], pos, 1, true );
            }
            else
            {
                moves << movesInDirection ( directions[SE], pos, 1, true, grid );
                moves << movesInDirection ( directions[SW], pos, 1, true, grid );
            }
            break;
        default:
            break;
    }
    return moves;
}

ChessRules::~ChessRules()
{

}

Rules::Directions ChessRules::legalDirections ( PieceType type )
{
    switch ( type )
    {
        case Queen:
        case King:
        case Knight:
            return AllDirections;

        case Pawn:
            return N;

        case Rook:
            return LineDirections;

        case Bishop:
            return DiagDirections;
        default:
            return None;
    }
}


bool ChessRules::isAttacked ( const Knights::Pos& pos, Color color, Grid* grid )
{
    if ( !grid )
    {
        grid = m_grid;
    }
    Grid::const_iterator it = grid->constBegin();
    Grid::const_iterator end = grid->constEnd();
    for ( ; it != end; ++it )
    {
        if ( it.value() && it.value()->color() != color && legalAttackMoves ( it.key(), grid ).contains ( Move ( it.key(), pos ) ) )
        {
            return true;
        }
    }
    return false;
}

bool ChessRules::isAttacking ( const Knights::Pos& attackingPos )
{
    const Color pieceColor = m_grid->value ( attackingPos )->color();
    const Color kingColor = oppositeColor ( pieceColor );
    return legalAttackMoves ( attackingPos, m_grid ).contains ( Move ( attackingPos, kingPos[kingColor] ) );
}


QList<Move> ChessRules::movesInDirection ( const Knights::Pos& dir, const Knights::Pos& pos, int length, bool attackYours, Grid* grid )
{
    if ( !grid )
    {
        grid = m_grid;
    }
    QList<Move> list;
    int num = 0;
    for ( Pos n = pos + dir; n.first > 0 && n.first < 9 && n.second > 0 && n.second < 9 && num < length; n += dir )
    {
        if ( !grid->contains ( n ) )
        {
            list << Move ( pos, n );
        }
        else if ( attackYours || grid->value ( n )->color() != grid->value ( pos )->color() )
        {
            list << Move ( pos, n, Move::Take );
            break;
        }
        else
        {
            break;
        }
        num++;
    }
    return list;
}

void ChessRules::checkSpecialFlags ( Move& move )
{
    Piece* p = m_grid->value ( move.from() );
    move.setFlags ( 0 );
    move.setFlag ( Move::Take, m_grid->contains ( move.to() ) );
    if ( p->pieceType() == King && length ( move ) == 2 )
    {
        // It's castling
        move.setFlag ( Move::Castle, true );
        int line = move.to().second;
        Move rookMove;
        rookMove.setTo ( ( move.from() + move.to() ) / 2 );
        if ( move.to().first > move.from().first )
        {
            rookMove.setFrom ( 8, line );
        }
        else
        {
            rookMove.setFrom ( 1, line );
        }
        move.setAdditionalMoves ( QList<Move>() << rookMove );
        kDebug() << move.additionalMoves().size();
    }
    else
    {
        if ( p->pieceType() == Pawn )
        {
            // Promotion?
            if ( ( p->color() == White && move.to().second == 8 ) || ( p->color() == Black && move.to().second == 1 ) )
            {
                move.setFlag ( Move::Promote, true );
            }
            Pos delta = move.to() - move.from();
            // En Passant?
            if ( delta.first != 0 && !m_grid->contains ( move.to() ) )
            {
                move.setFlag ( Move::EnPassant, true );
                Pos capturedPos = move.from() + Pos ( delta.first, 0 );
                move.setAdditionalCaptures ( QList<Pos>() << capturedPos );
            }
        }
    }
}

int ChessRules::length ( const Knights::Move& move )
{
    Pos delta = move.to() - move.from();
    return qMax ( qAbs ( delta.first ), qAbs ( delta.second ) );
}

QList< Move > ChessRules::pawnMoves ( const Knights::Pos& pos )
{
    QList<Move> list;
    Pos forwardDirection;
    QList<Pos> captureDirections;
    QList<Pos> enPassantDirections;
    enPassantDirections << directions[E] << directions[W];
    int baseLine = 0;
    if ( m_grid->value ( pos )->color() == White )
    {
        forwardDirection = directions[N];
        captureDirections << directions[NE] << directions[NW];
        baseLine = 2;
    }
    else
    {
        forwardDirection = directions[S];
        captureDirections << directions[SE] << directions[SW];
        baseLine = 7;
    }
    // Normal forward moves
    if ( !m_grid->contains ( pos + forwardDirection ) )
    {
        list << Move ( pos, pos + forwardDirection );
        // Double forward moves
        if ( pos.second == baseLine && !m_grid->contains ( pos + 2*forwardDirection ) )
        {
            list << Move ( pos, pos + 2*forwardDirection );
        }
    }
    // Normal copturing
    foreach ( const Pos& captureDir, captureDirections )
    {
        if ( m_grid->contains ( pos + captureDir ) && m_grid->value ( pos + captureDir )->color() != m_grid->value ( pos )->color() )
        {
            list << Move ( pos, pos + captureDir, Move::Take );
        }
    }
    return list;
}

void ChessRules::moveMade ( const Knights::Move& m )
{
    m_enPassantMoves.clear();
    switch ( m_grid->value ( m.to() )->pieceType() )
    {
            // For Kings and Rook, we track their movement for Castling
        case King:
            kDebug() << "King moved to" << m.to();
            kingMoved[m_grid->value ( m.to() )->color() ] = true;
            kingPos[m_grid->value ( m.to() )->color() ] = m.to();
            break;
        case Rook:
            if ( m.from() == kingRookStartPos[White] )
            {
                kingRookMoved[White] = true;
            }
            else if ( m.from() == queenRookStartPos[White] )
            {
                queenRookMoved[White] = true;
            }
            else if ( m.from() == kingRookStartPos[Black] )
            {
                kingRookMoved[Black] = true;
            }
            else if ( m.from() == queenRookStartPos[Black] )
            {
                queenRookMoved[Black] = true;
            }
            break;

            // We track Pawns for en-passant
        case Pawn:
            if ( length ( m ) == 2 )
            {
                Pos mid = ( m.to() + m.from() ) / 2;
                foreach ( Direction dir, QList<Direction>() << W << E )
                if ( m_grid->contains ( m.to() + directions[dir] ) )
                {
                    Move enPassantMove;
                    enPassantMove.setFrom ( m.to() + directions[dir] );
                    enPassantMove.setTo ( mid );
                    enPassantMove.setFlag ( Move::EnPassant, true );
                    m_enPassantMoves << enPassantMove;
                }
            }
        default:
            break;
    }
    MoveData data;
    data.m = m;
    moveHistory << data;
}

QList< Move > ChessRules::castlingMoves ( const Knights::Pos& pos )
{
    QList<Move> moves;
    Color color = m_grid->value ( pos )->color();
    if ( kingMoved[color] )
    {
        return QList<Move>();
    }
    if ( !queenRookMoved[color] && isPathClearForCastling ( pos, queenRookStartPos[color] ) )
    {
        Move m;
        m.setFlag ( Move::Castle, true );
        m.setAdditionalMoves ( QList<Move>() << Move ( queenRookStartPos[color], pos + directions[W] ) );
        m.setFrom ( pos );
        m.setTo ( pos + 2*directions[W] );
        moves << m;
    }
    if ( !kingRookMoved[color] && isPathClearForCastling ( pos, kingRookStartPos[color] ) )
    {
        Move m;
        m.setFlag ( Move::Castle, true );
        m.setAdditionalMoves ( QList<Move>() << Move ( kingRookStartPos[color], pos + directions[E] ) );
        m.setFrom ( pos );
        m.setTo ( pos + 2*directions[E] );
        moves << m;
    }
    return moves;
}

bool ChessRules::isPathClearForCastling ( const Pos& kingPos, const Pos& rookPos )
{
    // This is called only from castlingMoves(), so we can assume it's about castling
    if ( rookPos == kingPos || !m_grid->contains( kingPos ) )
    {
        return true;
    }
    Pos delta = rookPos - kingPos;
    Pos dir;
    if ( delta.first == 0 || delta.second == 0 )
    {
        dir = delta / abs ( delta.first + delta.second );
    }
    else if ( delta.first == delta.second || delta.first == -delta.second )
    {
        dir = delta / abs ( delta.first );
    }
    else
    {
        return false;
    }
    for ( Pos p = kingPos + dir; p != rookPos; p += dir )
    {
        if ( m_grid->contains ( p ) )
        {
            return false;
        }
    }

    Color kingColor = m_grid->value ( kingPos )->color();
    for ( int i = 0; i <= 2; ++i )
    {
        // Only the squares the King passes through must be safe
        if ( isAttacked ( kingPos + i * dir, kingColor ) )
        {
            return false;
        }
    }

    return true;
}
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
