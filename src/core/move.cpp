/***************************************************************************
    File                 : move.cpp
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
#include "move.h"
#include "knightsdebug.h"

#include <QTime>
#include <QRegExp>
#include <QRegularExpression>

namespace Knights {
class MovePrivate : public QSharedData {
public:
	MovePrivate();

	Pos from;
	Pos to;

	QList<Move> extraMoves;

	PieceType promotedType;
	Move::Flags flags;

	Move::Notation notationType;
	QString string;

	PieceDataMap addedPieces;
	PieceDataMap removedPieces;

	QTime time;
	PieceData pieceData;

	QMap<Move::Notation, QString> notationStrings;
};

MovePrivate::MovePrivate()
	: from(Pos() ),
	  to(Pos() ),
	  promotedType(NoType ),
	  flags(Move::None ),
	  notationType( Move::NoNotation) {
}


Move Move::castling(Move::CastlingSide side, Color color) {
	const int rank = (color == White) ? 1 : 8;
	const int rookFile = (side == QueenSide) ? 1 : 8;
	const int kingDestinationFile = (side == QueenSide) ? 3 : 7;
	const int rookDestinationFile = (side == QueenSide) ? 4 : 6;

	Move m;
	m.setFrom(5, rank);
	m.setTo(kingDestinationFile, rank);
	m.setFlag(Castle, true);

	Move rookMove;
	rookMove.setFrom(rookFile, rank);
	rookMove.setTo(rookDestinationFile, rank);
	rookMove.setFlag(Forced, true);
	m.setAdditionalMoves(QList<Move>() << rookMove);

	QLatin1String str((side == QueenSide) ? "O-O-O" : "O-O");
	m.setStringForNotation(Coordinate, str);
	m.setStringForNotation(Algebraic, str);
	m.setStringForNotation(LongAlgebraic, str);

	return m;
}

Move::Move() : d(new MovePrivate) {

}

Move::Move(Pos from, Pos to, Move::Flags flags) : d(new MovePrivate) {
	setFrom(from);
	setTo(to);
	setFlags(flags);
}

Move::Move(QString string) : d(new MovePrivate) {
	setString(string);
}

Move::Move(const Move& other) = default;

Move::~Move() = default;

void Move::operator=(const Move& other) {
	d = other.d;
}

bool Move::flag(Move::MoveFlag flag) const {
	return d->flags & flag;
}

Move::Flags Move::flags() const {
	return d->flags;
}

void Move::setFlag(Move::MoveFlag flag, bool value) {
	if(value )
		d->flags |= flag;
	else
		d->flags &= ~flag;

	if(flag == Forced) {
		for(QList<Move>::iterator it = d->extraMoves.begin(); it != d->extraMoves.end(); ++it)
			it->setFlag(flag, value);
	}
}

void Move::setFlags(Move::Flags flags) {
	d->flags = flags;
}

void Move::setString(QString string) {
	d->flags = None;
	d->extraMoves.clear();

	if( string.contains(QLatin1Char('x')) ) {
		setFlag(Take, true);
		string.remove(QLatin1Char('x'));
	}

	if( string.contains(QLatin1Char('+')) ) {
		setFlag(Check, true);
		string.remove(QLatin1Char('+'));
	}

	// In case of double check symbol ++, remove the last +
	if( string.contains(QLatin1Char('+')) )
		string.remove(QLatin1Char('+'));

	if( string.contains(QLatin1Char('#')) ) {
		setFlag(CheckMate, true);
		string.remove(QLatin1Char('#'));
	}

	string.remove(QLatin1Char('-'));
	string.remove(QLatin1Char(' '));
	string.remove(QLatin1Char('='));

	QRegExp longMoveTest = QRegExp(QLatin1String("^[a-h][1-8][a-h][1-8]"));
	if (longMoveTest.indexIn(string) > -1) {
		// Long move notation, can be directly converted to From and To
		d->notationType = Coordinate;
		setFrom( Pos(string.left(2)) );
		setTo( Pos(string.mid(2, 2)) );

		//check whether we need to promote a pawn (string looks like a7a8R for promotion to rook, etc.)
		if(string.size() > 4) {
			setFlag(Promote, true);
			setPromotedType( Piece::typeFromChar(string[4]) );
		}
	} else {
		// Short notation, just store the string for later
		d->string = string;
		d->notationType = Algebraic;
	}
}

QString Move::string(bool includeX) const {
	if(d->notationType == Algebraic)
		return d->string;

	QString str = from().string();
	if( (flags() & Take) && includeX )
		str += QLatin1Char('x');

	str += to().string();
	if(flags() & Promote) {
		str += QLatin1Char('=');
		str += Piece::charFromType(promotedType());
	}

	return str;
}


Pos Move::from() const {
	return d->from;
}

Pos Move::to() const {
	return d->to;
}

void Move::setFrom(const Pos& value) {
	d->from = value;
	d->notationType = Coordinate;
}

void Move::setFrom(int first, int second) {
	setFrom(Pos(first, second ));
}

void Move::setTo(const Pos& value) {
	d->to = value;
}

void Move::setTo(int first, int second) {
	d->to = Pos(first, second);
}

const QList< Move >& Move::additionalMoves() const {
	return d->extraMoves;
}

void Move::setAdditionalMoves(const QList< Move >& list) {
	d->extraMoves.clear();
	for (const Move& move : list) {
		Move m = move;
		m.setFlag(Move::Additional, true);
		d->extraMoves << m;
	}
}

PieceType Move::promotedType() const {
	return d->promotedType;
}

void Move::setPromotedType(PieceType type) {
	d->promotedType = type;
}

Move::Notation Move::notation() const {
	return d->notationType;
}

const PieceDataMap& Move::removedPieces() const {
	return d->removedPieces;
}

void Move::addRemovedPiece(const Pos& pos, const PieceData& data) {
	d->removedPieces.insert(pos, data);
}

void Move::setRemovedPieces(const PieceDataMap& map) {
	d->removedPieces = map;
}

const PieceDataMap& Move::addedPieces() const {
	return d->addedPieces;
}

void Move::addAddedPiece(const Pos& pos, const PieceData& data) {
	d->addedPieces.insert(pos, data);
}

void Move::setAddedPieces(const PieceDataMap& map) {
	d->addedPieces = map;
}

bool Move::operator==(Move other) const {
	return(d->from == other.from() && d->to == other.to());
}

Move Move::reverse() const {
	Move rev;
	rev.setFlags(d->flags);

	if(notation() == Algebraic) {
		// We can't reverse the move from the algebraic notation alone
		// Please don't call this with a short notation move
		qCCritical(LOG_KNIGHTS) << "Can't reverse the move from short algebraic notation";
		return rev;
	}
	rev.setTo(d->from);
	rev.setFrom(d->to);
	if(flags() & Promote )
		rev.setPromotedType(Pawn);

	QList<Move> additionalMoves;
	for (const Move& m : d->extraMoves)
		additionalMoves << m.reverse();
	rev.setAdditionalMoves(additionalMoves);

	rev.setAddedPieces(d->removedPieces);
	rev.setRemovedPieces(d->addedPieces);

	return rev;
}

bool Move::isValid() const {
	if(d->flags & Illegal )
		return false;
	if(d->notationType == Coordinate )
		return d->from.isValid() && d->to.isValid();
	else if(d->string.size() < 6) {
		if(d->string.contains(QRegularExpression(QLatin1String("[a-h][1-8]"))) || d->string.contains(QLatin1String("o-o")) )
			return true;
	}
	return false;
}

/**
* Sets the time at which this move was made.
* It is used to reset the clock after undoing.
*/
void Move::setTime(const QTime& time) {
	d->time = time;
}

/**
* Returns the time at which the move was made.
*/
QTime Move::time() {
	return d->time;
}

void Move::setPieceData(const PieceData& data) {
	d->pieceData = data;
}

PieceData Move::pieceData() const {
	return d->pieceData;
}

void Move::setStringForNotation(Move::Notation notation, const QString& string) {
	d->notationStrings [ notation ] = string;
}

QString Move::stringForNotation(Move::Notation notation) const {
	if(d->notationStrings.contains(notation) )
		return d->notationStrings [ notation ];
	else
		return QString();
}

}

QDebug operator<<(QDebug debug, const Knights::Move& move) {
	debug << move.string(true);
	return debug;
}
