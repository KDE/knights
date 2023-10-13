/***************************************************************************
    File                 : board.h
    Project              : Knights
    Description          : Game board (scene)
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

#ifndef KNIGHTSBOARD_H
#define KNIGHTSBOARD_H

#include "core/piece.h"
#include <KGameThemeProvider>

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

	explicit Board(KGameThemeProvider*, QObject* parent = nullptr);
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
	KGameGraphicsViewRenderer* renderer;
	KGameThemeProvider* m_themeProvider;

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

public Q_SLOTS:
	void movePiece(const Move&);
	void updateTheme();
	void updateGraphics();
	void changeDisplayedPlayer();
	void setCurrentColor(Color);
	void setPlayerColors(Colors);

Q_SIGNALS:
	void pieceMoved(const Move&);
	void activePlayerChanged(Color);
	void displayedPlayerChanged(Color);
	void centerChanged(const QPointF&);
};

}

#endif // KNIGHTSBOARD_H
