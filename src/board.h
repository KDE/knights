/***************************************************************************
    File                 : board.h
    Project              : Knights
    Description          : Game board (scene)
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

#ifndef KNIGHTSBOARD_H
#define KNIGHTSBOARD_H

#include "core/piece.h"
#include <KgThemeProvider>

#include <QGraphicsScene>
#include <QMap>
#include <QPointer>

class QDrag;

namespace Knights {

class Move;
class Item;
class Renderer;
class Pos;
class Rules;

class Board : public QGraphicsScene {
	Q_OBJECT
	Q_ENUMS(MarkerType)

public:
	enum MarkerType {
		LegalMove,
		Danger,
		Motion
	};

	explicit Board(KgThemeProvider*, QObject* parent = nullptr);
	~Board() override;

	void populate();
	static bool isInBoard(const Pos&);

private:
	Rules* m_rules;
	Grid m_grid;
	QMap<Pos, Item*> m_tiles;
	QList<Item*> m_borders;
	QList<Item*> m_notations;
	Item* m_background;
	bool m_displayBorders;
	bool m_displayNotations;
	KGameRenderer* renderer;
	KgThemeProvider* m_themeProvider;

	QPointer<QDrag> drag;
	bool m_dragActive;
	Piece* draggedPiece;
	Piece* selectedPiece;
	QPoint dragStartPoint;

	bool m_paused;
	int m_tileSize;
	QRectF m_boardRect;
	bool m_animated;
	QPointF m_draggedPos;
	QPointF m_dragStartPos;
	Color m_currentPlayer;
	Color m_displayedPlayer;
	Colors m_playerColors;
	QMap<Pos, Item*> markers;
	bool m_drawFrame;

	void addPiece(PieceType type, Color color, const Pos& pos);
	void addMarker(const Pos& pos, MarkerType type );
	void addMarker(const Pos& pos, const QString& spriteKey);
	void addTiles();

	Piece* pieceAt(const QPointF&);
	Pos mapFromScene(const QPointF&);
	QPointF mapToScene(Pos);
	void centerOnPos(Item* item, const Pos& pos, bool animated = true);
	void centerOnPos(Item* item, bool animated = true);
	void removeFrame();
	void centerAndResize(Item* item, QSize size, bool animated = true);
	PieceType getPromotedType();

protected:
	void mousePressEvent(QGraphicsSceneMouseEvent*) override;
	void mouseMoveEvent(QGraphicsSceneMouseEvent*) override;
	void mouseReleaseEvent(QGraphicsSceneMouseEvent*) override;
	void dropEvent(QGraphicsSceneDragDropEvent*) override;
	void dragEnterEvent(QGraphicsSceneDragDropEvent*) override;
	void dragMoveEvent(QGraphicsSceneDragDropEvent*) override;
	void dragLeaveEvent(QGraphicsSceneDragDropEvent*) override;

public slots:
	void movePiece(const Move&);
	void updateTheme();
	void updateGraphics();
	void changeDisplayedPlayer();
	void setCurrentColor(Color);
	void setPlayerColors(Colors);

signals:
	void pieceMoved(const Move&);
	void activePlayerChanged(Color);
	void displayedPlayerChanged(Color);
	void centerChanged(const QPointF&);
};

}

#endif // KNIGHTSBOARD_H
