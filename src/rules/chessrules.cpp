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

#include "rules/chessrules.h"

#include "core/move.h"
#include <KDebug>
#include <QtCore/QMap>
#include <gamemanager.h>

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
    queenRookStartPos[Black] = Pos ( 1, 8 );
    
    kingRookStartPos[White] = Pos ( 8, 1 );
    kingRookStartPos[Black] = Pos ( 8, 8 );

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

Color ChessRules::winner()
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

PieceDataMap ChessRules::startingPieces ( )
{
    PieceDataMap pieces;
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

QList<Move> ChessRules::legalAttackMoves ( const Pos& pos, Grid* grid )
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


bool ChessRules::isAttacked ( const Pos& pos, Color color, Grid* grid )
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

bool ChessRules::isAttacking ( const Pos& attackingPos )
{
    Q_ASSERT(m_grid->contains(attackingPos));
    Q_ASSERT(m_grid->value(attackingPos));
    const Color pieceColor = m_grid->value ( attackingPos )->color();
    const Color kingColor = oppositeColor ( pieceColor );
    return legalAttackMoves ( attackingPos, m_grid ).contains ( Move ( attackingPos, kingPos[kingColor] ) );
}


QList<Move> ChessRules::movesInDirection ( const Pos& dir, const Pos& pos, int length, bool attackYours, Grid* grid )
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

void ChessRules::checkSpecialFlags ( Move* move, Color color )
{
    kDebug() << move->string() << colorName ( color );
    if ( !move->flag ( Move::Castle ) )
    {
        if ( move->notation() == Move::Coordinate )
        {
            QString str = move->string();
            move->setStringForNotation ( Move::Coordinate, str );
            changeNotation ( move, Move::Algebraic, color );
            move->setStringForNotation ( Move::Algebraic, move->string() );
            move->setString ( str );
        }
        else
        {
            move->setStringForNotation ( Move::Algebraic, move->string() );
            changeNotation ( move, Move::Coordinate, color );
            move->setStringForNotation ( Move::Coordinate, move->string() );
        }
    }
    
    Q_ASSERT ( move->notation() == Move::Coordinate || !move->isValid() || move->flag ( Move::Castle ) );
    
    Piece* p = m_grid->value ( move->from() );
    if ( !p )
    {
        kWarning() << "No piece at position" << move->from();
        move->setFlag(Move::Illegal, true);
        return;
    }
    if ( !legalMoves ( move->from() ).contains(*move) )
    {
        kWarning() << "Illegal move" << move;
        move->setFlag(Move::Illegal, true);
        return;
    }
    move->setPieceData ( qMakePair( p->color(), p->pieceType() ) );
    
    if (!move->flag ( Move::Castle ))
    {
        // The long algebraic notation can be constructed from the two above
        QString lan;
        if ( move->pieceData().second != Pawn )
        {
            lan += Piece::charFromType ( move->pieceData().second );
        }
        lan += move->from().string();
        if ( move->flag ( Move::Take ) )
        {
            lan += QLatin1Char('x');
        }
        else
        {
            lan += QLatin1Char('-');
        }
        lan += move->to().string();
        move->setStringForNotation ( Move::LongAlgebraic, lan );
    }

    move->setFlags ( move->flags() & ~(Move::Take | Move::Castle | Move::Check | Move::CheckMate | Move::EnPassant | Move::Promote) );
    if ( m_grid->contains ( move->to() ) )
    {
        Piece* p = m_grid->value ( move->to() );
        move->addRemovedPiece ( move->to(), qMakePair ( p->color(), p->pieceType() ) );
        move->setFlag ( Move::Take, true );
    }
    if ( p->pieceType() == King && length ( *move ) == 2 )
    {        
        kDebug() << "Castling";
        Move::CastlingSide side;
        if ( move->to().first > move->from().first )
        {
            side = Move::KingSide;
        }
        else
        {
            side = Move::QueenSide;
        }
        *move = Move::castling(side, color);
        Q_ASSERT ( move->additionalMoves().size() == 1 );
    }
    if (!move->flag ( Move::Castle ))
    {
        if ( p->pieceType() == Pawn )
        {
            // Promotion?
            if ( ( p->color() == White && move->to().second == 8 ) || ( p->color() == Black && move->to().second == 1 ) )
            {
                move->setFlag ( Move::Promote, true );
            }
            Pos delta = move->to() - move->from();
            // En Passant?
            if ( delta.first != 0 && !m_grid->contains ( move->to() ) )
            {
                move->setFlag ( Move::EnPassant, true );
                Pos capturedPos = move->from() + Pos ( delta.first, 0 );
                Piece* p = m_grid->value ( capturedPos );
                move->addRemovedPiece ( capturedPos, qMakePair ( p->color(), p->pieceType() ) );
            }
        }
    }
    
    /**
     * Check for check after the move has been made
     */
    Grid afterMoveGrid = *m_grid;
    afterMoveGrid.insert(move->to(), afterMoveGrid.take(move->from()));
    
    Grid::ConstIterator it = afterMoveGrid.constBegin();
    Grid::ConstIterator end = afterMoveGrid.constEnd();
    
    const Color kingColor = oppositeColor ( color );
    
    for ( ; it != end; ++it )
    {
        
        if ( it.value()->color() == color && 
            legalAttackMoves ( it.key(), &afterMoveGrid ).contains ( Move ( it.key(), kingPos[kingColor] ) ) )
        {
            move->setFlag ( Move::Check, true );
        }
    }

    if ( move->flag(Move::Take) )
    {
        Piece* p = m_grid->value ( move->to() );
        move->addRemovedPiece ( move->to(), qMakePair ( p->color(), p->pieceType() ) );
    }
}

int ChessRules::length ( const Move& move )
{
    Pos delta = move.to() - move.from();
    return qMax ( qAbs ( delta.first ), qAbs ( delta.second ) );
}

QList< Move > ChessRules::pawnMoves ( const Pos& pos )
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

void ChessRules::moveMade ( const Move& m )
{
    kDebug() << m.string();
    if ( !m_grid->contains(m.to()) )
    {
        kDebug() << *m_grid;
    }
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

QList< Move > ChessRules::castlingMoves ( const Pos& pos )
{
    // TODO: move from a model which permanently stores king's and rooks' move history
    // to account for undone moves. 
    QList<Move> moves;
    Color color = m_grid->value ( pos )->color();
    if ( hasKingMoved ( color ) )
    {
        return QList<Move>();
    }
    if ( !hasRookMoved ( color, Move::QueenSide ) && isPathClearForCastling ( pos, queenRookStartPos[color] ) )
    {
        moves << Move::castling ( Move::QueenSide, color );
    }
    if ( !hasRookMoved ( color, Move::KingSide ) && isPathClearForCastling ( pos, kingRookStartPos[color] ) )
    {
        moves << Move::castling ( Move::KingSide, color );
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

void ChessRules::changeNotation ( Move* move, Move::Notation notation, Color color )
{
    kDebug() << *move << color;
    if ( !move->isValid() ||  move->notation() == notation || move->flag ( Move::Castle ) )
    {
        return;
    }
    
    kDebug() << move->string();
    
    if ( notation == Move::Coordinate )
    {
        // Converting from Algebraic to Coordinate
        /**
         * Possible notations:
         * * e4 == Pe4 == pawn to e4
         * * Ne4 == Knight to e4
         * * Rbe1 == Rook from file b to e1
         * * cd4 == Pawn from file c to d4
         */
        QChar c;
        PieceType type = NoType;
        int file = -1;
        int rank = -1;
        bool found = false;
        bool fileSet = false;
        bool typeSet = false;
        bool rankSet = false;
        PieceType promoteType = NoType;
        
        QString s = move->string().remove( QLatin1Char('x') ).remove( QLatin1Char(':') ).remove( QLatin1Char('=') );
        if  ( s.size() < 2 )
        {
                kWarning() << "Unknown move notation" << move->string();
                move->setFlag ( Move::Illegal, true );
                return;
        }
        
        if ( QByteArray("KQBNRPkqbnrp").contains ( s.right(1).toLatin1() ) )
        {
            promoteType = Piece::typeFromChar ( s.right(1).at(0) );
            s.chop ( 1 );
            move->setPromotedType ( promoteType );
        }
        
        if ( !QByteArray("KQBNRP").contains ( s[0].toLatin1() ) )
        {
            s = QLatin1Char('P') + s;
        }
        
        type = Piece::typeFromChar ( s[0] );
        typeSet = true;
        s.remove ( 0, 1 );
        
        switch ( s.size() )
        {
            case 2:
                // Only destination square
                break;
                
            case 3:
                // Either starting file or rank
                if ( QByteArray("abcdefgh").contains( s[0].toLatin1() ) )
                {
                    file = Pos::numFromRow ( s[0] );
                    fileSet = true;
                }
                else if ( QByteArray("12345678").contains ( s[0].toLatin1() ) )
                {
                    rank = QString( s[0] ).toInt();
                    rankSet = true;
                }
                break;
                
            case 4:
                // Both starting file and rank
                file = Pos::numFromRow ( s[0] );
                fileSet = true;
                rank = QString ( s[1] ).toInt();
                rankSet = true;
                break;
        }
        move->setTo ( s.right(2) );
        
        kDebug() << "Conditions:";
        if ( typeSet ) kDebug() << "Type == " << Piece::charFromType(type);
        if ( fileSet ) kDebug() << "File == " << file;
        if ( rankSet ) kDebug() << "Rank == " << rank;

        for ( Grid::const_iterator it = m_grid->constBegin(); it != m_grid->constEnd(); ++it )
        {
            if ( it.value()->color() == color
                && ( !typeSet || it.value()->pieceType() == type )
                && ( !fileSet || it.key().first == file )
                && ( !rankSet || it.key().second == rank )
                && legalMoves(it.key()).contains( Move( it.key(), move->to() ) ) )
            {
                if ( found )
                {
                    kWarning() << "Found more than one possible move";
                    move->setFlag ( Move::Illegal, true );
                    return;
                }
                move->setFrom( it.key() );
                found = true;
            }
        }
        if ( !found )
        {
            kWarning() << "No possible moves found" << move->string();
            move->setFlag ( Move::Illegal, true );
            return;
        }
    }
    else 
    {
        // Converting from Coordinate to Algebraic
        
        QStringList possibilities;
        
        /*
         * We try (in this order):
         * Ne6, Nde6, N5e6, Nd5e6
         */
        const PieceType type = m_grid->value ( move->from() )->pieceType();
        const QString typeLetter = (type != Pawn) ? Piece::charFromType ( type ) : QString();
        QString end = move->to().string();
        
        if ( move->flag ( Move::Take ) )
        {
            end = QLatin1Char('x') + end;
        }
        
        if ( move->flag ( Move::Promote ) )
        {
            end += QLatin1Char('=') + Piece::charFromType( move->promotedType() );
        }
        
        possibilities << typeLetter + end;
        possibilities << typeLetter + Pos::row ( move->from().first ) + end;
        possibilities << typeLetter + QString::number ( move->from().second ) + end;
        possibilities << typeLetter + move->from().string() + end;
                
        kDebug() << possibilities;
        
        foreach ( const QString& text, possibilities )
        {
            Move m ( text );
            checkSpecialFlags ( &m, color );
            if ( m.isValid() )
            {
                move->setString ( text );
                break;
            }
        }
        
    }
    
    kDebug() << move->string();
    
    Q_ASSERT ( move->notation() == notation );
}

bool ChessRules::hasKingMoved(Color color)
{
    PieceData data = qMakePair ( color, King );
    foreach ( const Move& move, Manager::self()->moveHistory() )
    {
        if ( move.pieceData() == data )
        {
            return true;
        }
    }
    return false;
}

bool ChessRules::hasRookMoved(Color color, Move::CastlingSide side)
{
    PieceData data = qMakePair ( color, Rook );
    Pos rookPos = ( side == Move::KingSide ) ? kingRookStartPos[color] : queenRookStartPos[color];
    foreach ( const Move& move, Manager::self()->moveHistory() )
    {
        if ( move.pieceData() == data && move.from() == rookPos )
        {
            return true;
        }
    }
    return false;
}


// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
