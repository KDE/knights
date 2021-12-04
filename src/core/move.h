/***************************************************************************
    File                 : move.h
    Project              : Knights
    Description          : Class representing a chess move
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2009-2011 Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/

#ifndef KNIGHTS_MOVE_H
#define KNIGHTS_MOVE_H

#include "piece.h"

#include <QSharedDataPointer>

namespace Knights {
class MovePrivate;

class Move {
public:

	typedef QList<Move> List;

	enum MoveFlag {
		None = 0x000,
		Take = 0x001,
		Promote = 0x002,
		Castle = 0x004,
		EnPassant = 0x008,
		Check = 0x010,
		CheckMate = 0x020,
		Additional = 0x040,
		Forced = 0x080,
		Illegal = 0x100
	};

	enum CastlingSide {
		QueenSide,
		KingSide
	};

	enum Notation {
		NoNotation, /**< No string has been set */
		Algebraic, /**< It omits the starting file and rank of the piece, unless it is necessary to disambiguate the move. */
		Coordinate, /**< specifies the starting and end position */
		LongAlgebraic /**< specifies almost everything */
	};
	Q_DECLARE_FLAGS(Flags, MoveFlag)

	static Move castling(CastlingSide, Color);

	Move(Pos from, Pos to, Flags flags = None);
	explicit Move(QString);
	Move();
	Move(const Move& other);
	virtual ~Move();

	void operator=(const Move& other);

	Pos from() const;
	Pos to() const;
	QString string(bool includeX = true) const;
	Move reverse() const;

	void setFrom(const Pos&);
	void setFrom(int first, int second);
	void setTo(const Pos&);
	void setTo(int first, int second);
	void setString(QString);

	bool flag(Move::MoveFlag) const;
	Flags flags() const;
	void setFlag(MoveFlag, bool value);
	void setFlags(Flags);

	const QList<Move>& additionalMoves() const;
	void setAdditionalMoves(const QList<Move>&);

	PieceType promotedType() const;
	void setPromotedType(PieceType);

	const PieceDataMap& removedPieces() const;
	void setRemovedPieces(const PieceDataMap&);
	void addRemovedPiece(const Pos&, const PieceData&);

	const PieceDataMap& addedPieces() const;
	void setAddedPieces(const PieceDataMap&);
	void addAddedPiece(const Pos&, const PieceData&);

	Notation notation() const;

	bool operator==(Move other ) const;

	void setTime(const QTime&);
	QTime time();

	void setPieceData(const PieceData&);
	PieceData pieceData() const;

	bool isValid() const;

	void setStringForNotation(Notation, const QString&);
	QString stringForNotation(Notation) const;

private:
	QSharedDataPointer<MovePrivate> d;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(Move::Flags)
}

QDebug operator<<(QDebug debug, const Knights::Move &move);

Q_DECLARE_METATYPE(Knights::Move)
Q_DECLARE_METATYPE(Knights::Move::List)

#endif // KNIGHTS_MOVE_H
