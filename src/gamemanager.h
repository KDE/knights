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

#ifndef KNIGHTS_GAMEMANAGER_H
#define KNIGHTS_GAMEMANAGER_H

#include <core/piece.h>
#include <core/move.h>
#include "offerwidget.h"

#include <KGlobal>

#include <QObject>
#include <QTime>
#include <QStack>

class QAbstractListModel;
class KgDifficultyLevel;

namespace Knights {

  class Protocol;
  class Rules;
  class Move;
  class GameManagerPrivate;

  struct TimeControl
            {
                int moves;
                QTime baseTime;
                int increment;
                QTime currentTime;
            };

	    enum GameAction
	    {
	      ActionNone,
	      ActionDraw,
	      ActionUndo,
	      ActionAdjourn,
	      ActionAbort,
	      ActionPause,
	      ActionResume,
	      ActionOther
	    };

	    class Offer
    {
    public:
        GameAction action;
        int id;
	QString text;
        Color player;
        int numberOfMoves; // Only used for Takeback offers.
        
        void accept() const;
	void decline() const;
    };

class Manager : public QObject
{
  Q_OBJECT
public:
  static Manager* self();
    explicit Manager(QObject* parent = 0);
    virtual ~Manager();

    void setCurrentTime ( Color color, const QTime& time );


            /**
             * Sets the time control parameters in the same format as XBoard's @c level command works
             * @param color specifis to which player this setting will apply. If @p color is NoColor then both player use this setting.
             * @param moves the number of moves to be completed before @p baseTime runs out.
             * Setting this to 0 causes the timing to be incremental only
             * @param baseTime the time in minutes in which the player has to complete @p moves moves, or finish the game if @p moves is zero.
             * @param increment the time in seconds that is added to the player's clock for his every move.
             */
     void setTimeControl ( Color color, const TimeControl& control );
     TimeControl timeControl ( Color color ) const;
     bool timeControlEnabled ( Color color ) const;
     QTime timeLimit ( Color color );


            void changeActivePlayer();
            void setActivePlayer ( Color player );
            Color activePlayer() const;
	    bool isRunning();
	    
	    bool canRedo() const;
            bool isGameActive() const;
            bool canLocalMove() const;
            
    Rules* rules() const;
    void setRules ( Rules* rules );
        void reset();
	
	void startGame();
        QStack<Move> moveHistory() const;
            
private:
    void addMoveToHistory ( const Move& move );
    Move nextUndoMove();
    Move nextRedoMove();

    void startTime();
    void stopTime();
    
    void processMove(const Move& move);
    
    Protocol* local();
    bool getCustomDifficulty(int* depth, int* size);
    
private slots:
    void sendPendingMove();

protected:
    virtual void timerEvent(QTimerEvent* );

signals:
  void timeChanged ( Color color, const QTime& time );
  void undoPossible ( bool possible );
  void redoPossible ( bool possible );
  void pieceMoved ( const Move& move );
  void activePlayerChanged ( Color player );
  void initComplete();
  void notification ( const Offer& offer );
  void winnerNotify ( Color winner );
  
  void historyChanged();
  void playerNameChanged();

public slots:
  void moveByProtocol ( const Move& move );
  void moveByBoard ( const Move& move );
  void moveByExternalControl ( const Move& move );
  void protocolInitSuccesful();
  void gameOver ( Color winner );
  void resign();

  void sendOffer ( GameAction action, Color player = NoColor, int id = 0 );
  void sendOffer ( const Offer& offer );
  void setOfferResult ( int id, OfferAction result );
  
    void initialize();
    void undo();
    void redo();
    void offerDraw();
    void adjourn();
    void abort();
    
    void setTimeRunning(bool running); 
    
    void levelChanged ( const KgDifficultyLevel* level );
    void setDifficulty ( int searchDepth, int memorySize );

    void saveGameHistoryAs(const QString& filename);
    void loadGameHistoryFrom(const QString& filename);

private:
  GameManagerPrivate* d_ptr;
    Move pendingMove;
  Q_DECLARE_PRIVATE(GameManager)
  

};

}

#endif // KNIGHTS_GAMEMANAGER_H
