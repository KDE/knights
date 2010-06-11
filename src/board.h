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

#ifndef KCHESS_BOARD_H
#define KCHESS_BOARD_H

#include "core/piece.h"
#include "core/move.h"

#include <QtGui/QGraphicsScene>
#include <QtCore/QMap>
#include <QtCore/QSet>

class QGraphicsSvgItem;
class KGameTheme;
class KSvgRenderer;

namespace Knights {
class Pos;
class Rules;

class Board : public QGraphicsScene
{
    Q_OBJECT
public:
    Board(QObject* parent = 0);
    virtual ~Board();

    void populate();
    void addPiece( PieceType type, Color color, const Pos& pos);

    void setRuleSet(Rules* rules);

    static bool isInBoard(const Pos& pos);
    void setPlayerColors(const QList<Color>& colors);
    QList<Color> playerColors();
    void setCurrentColor(Color color);

private:
    Rules *m_rules;
    Grid m_grid;
    QMap<QGraphicsSvgItem*,QString> m_elementIdMap;
    QGraphicsSvgItem* m_boardItem;
    KGameTheme* theme;
    KSvgRenderer* svg;
    Piece* pieceAt(QPointF point);
    Pos mapFromScene(QPointF point);
    QPointF mapToScene(Pos pos);
    void changeCurrentPlayer();
    void centerOnPos( QGraphicsItem* item, const Pos& pos, bool animated = true);
    void repaintBoard();
    bool m_paused;
    qreal m_tileSize;
    QRectF m_boardRect;
    bool m_animated;
    Piece* m_draggedItem;
    QPointF m_draggedPos;
    QPointF m_dragStartPos;
    Color m_currentPlayer;
    Color m_displayedPlayer;
    QList<Color> m_playerColors;
    QList<QGraphicsSvgItem*> m_legalMarkers;

protected:
    virtual void mousePressEvent(QGraphicsSceneMouseEvent* e);
    virtual void dropEvent(QGraphicsSceneDragDropEvent* e);
    virtual void dragEnterEvent(QGraphicsSceneDragDropEvent* e);
    virtual void dragMoveEvent(QGraphicsSceneDragDropEvent* e);

public slots:
    void movePiece(Move m, bool changePlayer = true);
    void addMarker(const Pos& pos);
    void updateTheme();
        void setPaused(bool paused);

signals:
    void pieceMoved(Move m);
    void gameOver(Color winner);
    
    void activePlayerChanged(Color);
    void displayedPlayerChanged(Color);
};

}

#endif // KCHESS_BOARD_H
