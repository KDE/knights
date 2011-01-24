/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef KNIGHTS_GAMEMANAGER_H
#define KNIGHTS_GAMEMANAGER_H

#include <core/piece.h>
#include <QtCore/QObject>
#include <QtCore/QTime>

#include <KGlobal>

namespace Knights {

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

class Manager : public QObject
{
  Q_OBJECT
public:
  static Manager* self();
    explicit Manager(QObject* parent = 0);
    virtual ~Manager();

    void addMoveToHistory ( const Move& move );
    Move nextUndoMove();
    Move nextRedoMove();

    void startTime();
    void stopTime();
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
    void undo();
    void initialize();
    void redo();

    Rules* rules();

protected:
    virtual void timerEvent(QTimerEvent* );

signals:
  void timeChanged ( Color color, const QTime& time );
  void undoPossible ( bool possible );
  void redoPossible ( bool possible );
  void pieceMoved ( const Move& move );
  void activePlayerChanged ( Color player );
  void initComplete();

public slots:
  void moveByProtocol ( const Move& move );
  void moveByBoard ( const Move& move );
  void protocolInitSuccesful();
  void gameOver();
  void resign();

private:
  GameManagerPrivate* d_ptr;
  Q_DECLARE_PRIVATE(GameManager)
  

};

}

#endif // KNIGHTS_GAMEMANAGER_H
