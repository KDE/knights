/***************************************************************************
    File                 : gamemanager.h
    Project              : Knights
    Description          : Game manager
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2009-2011 by Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
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

#ifndef KNIGHTS_GAMEMANAGER_H
#define KNIGHTS_GAMEMANAGER_H

#include <core/piece.h>
#include <core/move.h>
#include "offerwidget.h"

#include <QTime>
#include <QStack>

class KgDifficultyLevel;

namespace Knights {

class Protocol;
class Rules;
class Move;
class GameManagerPrivate;

struct TimeControl {
	int moves;
	QTime baseTime;
	int increment;
	QTime currentTime;
};

enum GameAction {
	ActionNone,
	ActionDraw,
	ActionUndo,
	ActionAdjourn,
	ActionAbort,
	ActionPause,
	ActionResume,
	ActionOther
};

class Offer {
public:
	GameAction action;
	int id;
	QString text;
	Color player;
	int numberOfMoves; // Only used for Takeback offers.

	void accept() const;
	void decline() const;
};

class Manager : public QObject {
	Q_OBJECT
public:
	static Manager* self();
	explicit Manager(QObject* parent = 0);
	virtual ~Manager();

	void setCurrentTime(Color, const QTime&);
	void setTimeControl(Color, const TimeControl&);
	TimeControl timeControl(Color) const;
	bool timeControlEnabled(Color) const;
	QTime timeLimit(Color);

	void changeActivePlayer();
	void setActivePlayer(Color);
	Color activePlayer() const;
	bool isRunning();

	bool canRedo() const;
	bool isGameActive() const;
	bool canLocalMove() const;

	Rules* rules() const;
	void setRules(Rules*);
	void reset();

	void startGame();
	QStack<Move> moveHistory() const;

private:
	void addMoveToHistory(const Move&);
	Move nextUndoMove();
	Move nextRedoMove();

	void startTime();
	void stopTime();

	void processMove(const Move&);
	Protocol* local();

public slots:
	void levelChanged(const KgDifficultyLevel*);

	void moveByExternalControl(const Move&);
	void moveByBoard(const Move&);

	void sendOffer(GameAction action, Color player = NoColor, int id = 0);
	void sendOffer(const Offer&);
	void setOfferResult(int id, OfferAction result);

	void initialize();
	void undo();
	void redo();

	void loadGameHistoryFrom(const QString& filename);
	void saveGameHistoryAs(const QString& filename);

private slots:
	void moveByProtocol(const Move&);
	void protocolInitSuccesful();
	void gameOver(const Color);
	void resign();
	void offerDraw();
	void adjourn();
	void abort();
	void setTimeRunning(bool);
	void sendPendingMove();

private:
	GameManagerPrivate* d_ptr;
	Move pendingMove;
	Q_DECLARE_PRIVATE(GameManager)

protected:
	virtual void timerEvent(QTimerEvent*);

signals:
	void timeChanged(Color, const QTime&);
	void undoPossible(bool);
	void redoPossible(bool);
	void pieceMoved(const Move&);
	void activePlayerChanged(Color);
	void initComplete();
	void notification(const Offer& );
	void winnerNotify(Color);
	void historyChanged();
	void playerNameChanged();
};

}

#endif // KNIGHTS_GAMEMANAGER_H
