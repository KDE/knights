/***************************************************************************
    File                 : gamemanager.h
    Project              : Knights
    Description          : Game manager
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

#ifndef KNIGHTS_GAMEMANAGER_H
#define KNIGHTS_GAMEMANAGER_H

#include <core/piece.h>
#include <core/move.h>
#include "offerwidget.h"

#include <QTime>
#include <QStack>

class KGameDifficultyLevel;

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
	ActionResign,
	ActionOther
};

class Offer {
public:
	Offer() {
		action = ActionNone;
		id = 0;
		player = NoColor;
		numberOfMoves = 0;
	};
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
	explicit Manager(QObject* parent = nullptr);
	~Manager() override;

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
	void resign();
	QStack<Move> moveHistory() const;

private:
	void addMoveToHistory(const Move&);
	Move nextUndoMove();
	Move nextRedoMove();

	void startTime();
	void stopTime();

	void processMove(const Move&);
	Protocol* local();

public Q_SLOTS:
	void levelChanged(const KGameDifficultyLevel*);

	void moveByExternalControl(const Move&);
	void moveByBoard(const Move&);

	void sendOffer(GameAction action, Color player = NoColor, int id = 0);
	void sendOffer(const Offer&);
	void setOfferResult(int id, OfferAction result);

	void initialize();
	void pause(bool);
	void undo();
	void redo();

	void offerDraw();
	void adjourn();
	void abort();

	void loadGameHistoryFrom(const QString& filename);
	void saveGameHistoryAs(const QString& filename);

private Q_SLOTS:
	void moveByProtocol(const Move&);
	void protocolInitSuccesful();
	void gameOver(Color);
	void setTimeRunning(bool);
	void sendPendingMove();

private:
	GameManagerPrivate* d_ptr;
	Move pendingMove;
	Q_DECLARE_PRIVATE(GameManager)

protected:
	void timerEvent(QTimerEvent*) override;

Q_SIGNALS:
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
