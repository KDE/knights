/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "uciprotocol.h"
#include "gamemanager.h"
#include "knightsdebug.h"

#include <KProcess>
using namespace Knights;

UciProtocol::UciProtocol(QObject* parent): ComputerProtocol(parent),
	mWhiteTime(0),
	mBlackTime(0),
	mDifficulty(0) {

}

UciProtocol::~UciProtocol() {
	if ( mProcess && mProcess->isOpen() ) {
		write("quit");
		if ( !mProcess->waitForFinished ( 500 ) )
			mProcess->kill();
	}
}

Protocol::Features UciProtocol::supportedFeatures() {
	//TODO: UCI is stateless. Undo needs to be implemented completely in the client.
	return GameOver | Pause | Resign | SetDifficulty | AdjustDifficulty;
}

bool UciProtocol::parseStub(const QString& line) {
	Q_UNUSED(line);
	return false;
}

bool UciProtocol::parseLine(const QString& line) {
	if ( line.isEmpty() )
		return true;
	ChatWidget::MessageType type = ChatWidget::GreetMessage;
	qCDebug(LOG_KNIGHTS) << line;
	if ( line.startsWith ( QLatin1String("uciok") ) ) {
		type = ChatWidget::AccountMessage;
		write ( "isready" );
	} else if ( line.startsWith ( QLatin1String("readyok") ) ) {
		type = ChatWidget::AccountMessage;
		initComplete();
	} else if ( line.startsWith ( QLatin1String("id name ") ) ) {
		type = ChatWidget::AccountMessage;
		// Chop off the "id name " port, the remainder if the engine's name
		setPlayerName ( line.mid ( 8 ) );
	} else if ( line.startsWith ( QLatin1String("bestmove") ) ) {
		type = ChatWidget::MoveMessage;
		QStringList lst = line.split(QLatin1Char(' '));
		if ( lst.size() > 1 ) {
			Move m = Move ( lst[1] );
			mMoveHistory << m;
			Q_EMIT pieceMoved ( m );
		} else
			return false;
		if ( lst.size() > 3 && lst[2] == QLatin1String("ponder") )
			mPonderMove.setString ( lst[3] );
	}
	writeToConsole ( line, type );
	return true;
}

void UciProtocol::declineOffer(const Knights::Offer& offer) {
	Q_UNUSED(offer);
}

void UciProtocol::acceptOffer(const Knights::Offer& offer) {
	Q_UNUSED(offer);
}

void UciProtocol::makeOffer(const Knights::Offer& offer) {
	Q_UNUSED(offer);
}

void UciProtocol::startGame() {
	write ( "ucinewgame" );
	if ( color() == White )
		requestNextMove();
}

void UciProtocol::init() {
	startProgram();
	write("uci");
}

void UciProtocol::move(const Knights::Move& m) {
	mMoveHistory << m;
	requestNextMove();
}

void UciProtocol::requestNextMove() {
	QString str = QStringLiteral("position startpos");

	if ( !mMoveHistory.isEmpty() ) {
		str += QLatin1String(" moves");
		for ( const Move& move : qAsConst(mMoveHistory) ) {
			QString moveString = QLatin1Char(' ') + move.from().string() + move.to().string();
			if ( move.promotedType() )
				moveString += Piece::charFromType ( move.promotedType() ).toLower();
			str += moveString;
		}
	}
	write ( str );

	QString goString = QStringLiteral("go");

	//TODO: mDifficulty doesn't seem to be properly set nowhere
	if ( mDifficulty )
		goString += QLatin1String(" depth ") + QString::number ( mDifficulty );

	if ( Manager::self()->timeControlEnabled(NoColor) ) {
		goString += QLatin1String(" wtime ") + QString::number ( mWhiteTime );
		goString += QLatin1String(" btime ") + QString::number ( mBlackTime );

		int winc = Manager::self()->timeControl ( White ).increment;
		if ( winc )
			goString += QLatin1String(" winc ") + QString::number ( winc * 1000 );
		int binc = Manager::self()->timeControl ( Black ).increment;
		if ( winc )
			goString += QLatin1String(" binc ") + QString::number ( binc * 1000 );

		int moves = Manager::self()->timeControl ( NoColor ).moves;
		if (moves > 0) {
			int movesToGo = mMoveHistory.size() % moves;
			if ( movesToGo > 0 )
				goString += QLatin1String(" movestogo ") + QString::number ( movesToGo );
		}
	}

	write ( goString );
}

void UciProtocol::changeCurrentTime(Color color, const QTime& time) {
	int msecs = QTime().msecsTo ( time );
	switch ( color ) {
	case White:
		mWhiteTime = msecs;
		break;

	case Black:
		mBlackTime = msecs;
		break;

	default:
		mBlackTime = msecs;
		mWhiteTime = msecs;
		break;
	}
}

void UciProtocol::setDifficulty(int depth, int memory) {
	mDifficulty = depth;
	write ( QLatin1String("setoption name Hash value ") + QString::number ( memory ) );
}

