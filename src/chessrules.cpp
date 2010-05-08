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

#include "chessrules.h"

#include "core/move.h"

#include <KDebug>

#include <QtCore/QMap>

using namespace Knights;


ChessRules::ChessRules()
{
    lineDirs.insert(E, Pos(1,0));
    lineDirs.insert(W, Pos(-1,0));
    lineDirs.insert(S, Pos(0,-1));
    lineDirs.insert(N, Pos(0,1));

    diagDirs.insert(NW, lineDirs[N] + lineDirs[W]);
    diagDirs.insert(NE, lineDirs[N] + lineDirs[E]);
    diagDirs.insert(SW, lineDirs[S] + lineDirs[W]);
    diagDirs.insert(SE, lineDirs[S] + lineDirs[E]);

    directions.unite(lineDirs).unite(diagDirs);

    knightDirs << Pos(1,2);
    knightDirs << Pos(-1,2);
    knightDirs << Pos(1,-2);
    knightDirs << Pos(-1,-2);
    knightDirs << Pos(2,1);
    knightDirs << Pos(-2,1);
    knightDirs << Pos(2,-1);
    knightDirs << Pos(-2,-1);
    
    queenRookStartPos[Piece::White] = Pos(1,1);
    queenRookMoved[Piece::White] = false;
    kingRookStartPos[Piece::White] = Pos(8,1);
    kingRookMoved[Piece::Black] = false;
    queenRookStartPos[Piece::Black] = Pos(1,8);
    queenRookMoved[Piece::Black] = false;
    kingRookStartPos[Piece::Black] = Pos(8,8);
    kingRookMoved[Piece::Black] = false;

    kingMoved[Piece::White] = false;
    kingMoved[Piece::Black] = false;
    
    kingPos[Piece::White] = Pos(5,1);
    kingPos[Piece::Black] = Pos(5,8);
}


bool ChessRules::hasLegalMoves ( Piece::Color color )
{
  foreach(const Pos& pos, m_grid->keys())
  {
    if (m_grid->value(pos) && m_grid->value(pos)->color() == color && !legalMoves(pos).isEmpty() )
    {
      return true;
    }
  }
  return false;
}

Knights::Piece::Color ChessRules::winner()
{
  if (isKingAttacked(Piece::Black) && !hasLegalMoves(Piece::Black))
  {
    return Piece::White;
  }
  if (isKingAttacked(Piece::White) && !hasLegalMoves(Piece::White))
  {
    return Piece::Black;
  }
  return Piece::NoColor;
}

QMap< Pos, Knights::Piece::PieceType > ChessRules::startingPieces(Knights::Piece::Color color)
{
    if (color != Piece::White && color != Piece::Black)
    {
        return QMap<Pos,Piece::PieceType>();
    }
    QMap<Pos,Piece::PieceType> pieces;
    int baseLine = 0;
    int pawnLine = 0;
    const int kingRow = 5;
    const int queenRow = 4;
    if (color == Piece::White)
    {
        baseLine = 1;
        pawnLine = 2;
    }
    else
    {
        baseLine = 8;
        pawnLine = 7;
    }
    pieces.insert(Pos(kingRow,baseLine), Piece::King);
    pieces.insert(Pos(queenRow,baseLine), Piece::Queen);
    for (int i = 0; i < 2; ++i)
    {
        pieces.insert(Pos(1+i*7,baseLine), Piece::Rook);
        pieces.insert(Pos(2+i*5,baseLine), Piece::Knight);
        pieces.insert(Pos(3+i*3,baseLine), Piece::Bishop);
    }
    for (int i = 1; i < 9; ++i)
    {
        pieces.insert(Pos(i,pawnLine), Piece::Pawn);
    }
    return pieces;
}


bool ChessRules::isKingAttacked ( Piece::Color color, Grid* grid)
{
    return isAttacked(kingPos[color], color, grid);
}


QList<Move> ChessRules::legalMoves(Pos pos)
{
    QList<Move> moves;
    bool isKingMoving = false;
    Piece::Color color = m_grid->value(pos)->color();
    switch (m_grid->value(pos)->pieceType())
    {
    case Piece::King:
        foreach(const Pos& d, directions)
        {
          foreach(const Move& m, movesInDirection(d, pos, 1))
          {
            moves << m;
          }
        }
        isKingMoving = true;
        moves << castlingMoves(pos);
        break;
    case Piece::Queen:
        foreach(const Pos& d, directions)
        {
            moves << movesInDirection(d,pos);
        }
        break;
    case Piece::Bishop:
        foreach(const Pos& d, diagDirs)
        {
          moves << movesInDirection(d,pos);
        }
        break;
    case Piece::Rook:
        foreach(const Pos& d, lineDirs)
        {
            moves.append(movesInDirection(d,pos));
        }
        break;
    case Piece::Knight:
        foreach(const Pos& d, knightDirs)
        {
          if (!Board::isInBoard(pos + d))
          {
            continue;
          }
          if (!m_grid->contains(pos + d))
          {
            moves << Move(pos, pos + d);
          }
          else if (m_grid->value(pos + d)->color() != color)
          {
            moves << Move(pos, pos + d, Move::Take);
          }
        }
        break;
    case Piece::Pawn:
        moves << pawnMoves(pos);
        foreach(const Move& m, m_enPassantMoves)
        {
          if (m.from() == pos)
          {
            moves << m;
          }
        }
        break;
    }
    foreach(const Move& m, moves)
    {
      Grid t_grid = *m_grid;
      t_grid.insert(m.to(), t_grid.take(pos));
      if ( ( isKingMoving && isAttacked(m.to(), color, &t_grid) ) || ( !isKingMoving && isAttacked(kingPos[color], color, &t_grid) ) )
      {
        moves.removeAll(m);
      }
    }
    return moves;
}


QList<Move> ChessRules::legalAttackMoves ( Pos pos, Grid* grid )
{
    if (!grid)
    {
	grid = m_grid;
    }
    // Defending your pieces is counted here, hence all there 'true's in movesInDirection calls
    QList<Move> moves;
    switch (grid->value(pos)->pieceType())
    {
	case Piece::King:
	    foreach(const Pos& d, directions)
	    {
		moves << movesInDirection(d, pos, 1, true, grid);
	    }
	    break;
	case Piece::Queen:
	    foreach(const Pos& d, directions)
	    {
		moves << movesInDirection(d,pos, 8, true, grid);
	    }
	    break;
	case Piece::Bishop:
	    foreach(const Pos& d, diagDirs)
	    {
		moves << movesInDirection(d,pos,8,true,grid);
	    }
	    break;
	case Piece::Rook:
	    foreach(const Pos& d, lineDirs)
	    {
		moves.append(movesInDirection(d,pos,8,true,grid));
	    }
	    break;
	case Piece::Knight:
	    foreach(const Pos& d, knightDirs)
	    {
		if (Board::isInBoard(pos + d))
		{
		    moves << Move(pos, pos + d);
		}
	    }
	    break;
	case Piece::Pawn:
	    if (grid->value(pos)->color() == Piece::White)
	    {
		moves << movesInDirection(directions[NE], pos, 1, true,grid);
		moves << movesInDirection(directions[NW], pos, 1, true);
	    }
	    else
	    {
		moves << movesInDirection(directions[SE], pos, 1, true,grid);
		moves << movesInDirection(directions[SW], pos, 1, true,grid);
	    }
	    break;
    }
    return moves;
}


ChessRules::~ChessRules()
{

}

Rules::Directions ChessRules::legalDirections(Piece::PieceType type)
{
    switch (type)
    {
    case Piece::Queen:
    case Piece::King:
    case Piece::Knight:
        return AllDirections;

    case Piece::Pawn:
        return N;

    case Piece::Rook:
        return LineDirections;

    case Piece::Bishop:
        return DiagDirections;
    }
    return None;
}


bool ChessRules::isAttacked(Pos pos, Piece::Color color, Grid* grid)
{
    if (!grid)
    {
	grid = m_grid;
    }
    foreach(const Pos& attackingPos, grid->keys())
    {
	if (grid->value(attackingPos) && grid->value(attackingPos)->color() != color && legalAttackMoves(attackingPos, grid).contains(Move(attackingPos, pos)))
	{
		return true;
	}
    }
    return false;
}

QList<Move> ChessRules::movesInDirection(Pos dir, Pos pos, int length, bool attackYours, Grid* grid)
{
    if (!grid)
    {
	grid = m_grid;
    }
    QList<Move> list;
    int num = 0;
    for (Pos n = pos + dir; n.first > 0 && n.first < 9 && n.second > 0 && n.second < 9 && num < length; n += dir)
    {
        if (!grid->contains(n))
        {
            list.append(Move(pos,n));
        }
        else if (attackYours || grid->value(n)->color() != grid->value(pos)->color())
        {
            list.append(Move(pos,n,Move::Take));
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

void ChessRules::checkSpecialFlags(Move* move)
{
    Piece* p = m_grid->value(move->from());
    move->setFlags(0);
    move->setFlag(Move::Take, m_grid->contains(move->to()));
    if (p->pieceType() == Piece::King && length(move) == 2)
    {
      // It's castling
        move->setFlag(Move::Castle, true);
        int line = move->to().second;
        Move rookMove;
        rookMove.setTo((move->from() + move->to())/2);
        if (move->to().first > move->from().first)
        {
          rookMove.setFrom(8, line);
        }
        else
        {
          rookMove.setFrom(1, line);
        }
        move->setAdditionalMoves(QList<Move>() << rookMove);
        kDebug() << move->additionalMoves().size();
    }
    else {
        if (p->pieceType() == Piece::Pawn)
        {
            // Promotion?
            if ((p->color() == Piece::White && move->to().second == 8) || (p->color() == Piece::Black && move->to().second == 1))
            {
                move->setFlag(Move::Promote, true);
            }
            Pos delta = move->from() - move->to();
            // En Passant?
            if ( delta.first != 0 && !m_grid->contains(move->to()))
            {
              move->setFlag(Move::EnPassant, true);
              Pos capturedPos = move->from() + Pos(delta.first, 0);
              move->setAdditionalCaptures(QList<Pos>() << capturedPos);
            }
        }
    }
}

int ChessRules::length(Move* move)
{
  Pos delta = move->to() - move->from();
  return qMax(qAbs(delta.first), qAbs(delta.second));
}

QList< Move > ChessRules::pawnMoves(Pos pos)
{
  QList<Move> list;
  Pos forwardDirection;
  QList<Pos> captureDirections;
  QList<Pos> enPassantDirections;
  enPassantDirections << directions[E] << directions[W];
  int baseLine = 0;
  if (m_grid->value(pos)->color() == Piece::White)
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
  if (!m_grid->contains(pos + forwardDirection) )
  {
    list << Move(pos, pos + forwardDirection);
    // Double forward moves
    if (pos.second == baseLine && !m_grid->contains(pos + 2*forwardDirection))
    {
      list << Move(pos, pos + 2*forwardDirection);
    }
  }
  // Normal copturing
  foreach (const Pos& captureDir, captureDirections)
  {
    if (m_grid->contains(pos + captureDir) && m_grid->value(pos + captureDir)->color() != m_grid->value(pos)->color())
    {
      list << Move(pos, pos + captureDir, Move::Take);
    }
  }
  return list;
}

void ChessRules::moveMade(Move move)
{
  kDebug() << m_grid->value(move.to())->pieceType();
  m_enPassantMoves.clear();
  switch (m_grid->value(move.to())->pieceType())
  {
    // For Kings and Rook, we track their movement for Castling
    case Piece::King:
      kDebug() << "King moved to" << move.to();
      kingMoved[m_grid->value(move.to())->color()] = true;
      kingPos[m_grid->value(move.to())->color()] = move.to();
      break;
    case Piece::Rook:
      if (move.from() == kingRookStartPos[Piece::White]) {
        kingRookMoved[Piece::White] = true;
      }
      else if (move.from() == queenRookStartPos[Piece::White]) {
        queenRookMoved[Piece::White] = true;
      }
      else if (move.from() == kingRookStartPos[Piece::Black]) {
        kingRookMoved[Piece::Black] = true;
      }
      else if (move.from() == queenRookStartPos[Piece::Black]) {
        queenRookMoved[Piece::Black] = true;
      }
      break;

      // We track Pawns for en-passant
    case Piece::Pawn:
      if (length(&move) == 2)
      {
        Pos mid = (move.to() + move.from())/2;
        foreach (Direction dir, QList<Direction>() << W << E)
        if (m_grid->contains(move.to() + directions[dir]))
        {
          Move m;
          move.setFrom(move.to() + directions[dir]);
          move.setTo(mid);
          move.setFlag(Move::EnPassant, true);
          m_enPassantMoves << m;
        }
      }
    default:
      break;
  }
  MoveData data;
  data.m = move;
  moveHistory << data;
}

QList< Move > ChessRules::castlingMoves(Pos pos)
{
  QList<Move> moves;
  Piece::Color color = m_grid->value(pos)->color();
  if (kingMoved[color])
  {
    return QList<Move>();
  }
  if (!queenRookMoved[color] && isPathClear(pos, queenRookStartPos[color]))
  {
    Move m;
    m.setFlag(Move::Castle, true);
    m.setAdditionalMoves(QList<Move>() << Move(queenRookStartPos[color], pos + directions[W]));
    m.setFrom(pos);
    m.setTo(pos + 2*directions[W]);
    moves << m;
  }
  if (!kingRookMoved[color] && isPathClear(pos, kingRookStartPos[color]))
  {
    Move m;
    m.setFlag(Move::Castle, true);
    m.setAdditionalMoves(QList<Move>() << Move(kingRookStartPos[color], pos + directions[E]));
    m.setFrom(pos);
    m.setTo(pos + 2*directions[E]);
    moves << m;
  }
  return moves;
}

bool Knights::ChessRules::isPathClear(Pos from, Pos to)
{
  if (to == from)
  {
    return true;
  }
  Pos delta = to - from;
  Pos dir;
  if (delta.first == 0 || delta.second == 0)
  {
    dir = delta/abs(delta.first + delta.second);
  }
  else if (delta.first == delta.second || delta.first == -delta.second)
  {
    dir = delta/abs(delta.first);
  }
  else
  {
    return false;
  }
  for (Pos p = from + dir; p != to; p += dir)
  {
    if (m_grid->contains(p))
    {
      return false;
    }
  }
  return true;
}
