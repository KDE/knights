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

#ifndef KNIGHTS_BOARD_H
#define KNIGHTS_BOARD_H

#include "core/piece.h"
#include "core/move.h"

#include <KgThemeProvider>

#include <QtGui/QGraphicsScene>
#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QPointer>

class QDrag;


namespace Knights
{

    class Item;
    class Renderer;
    class Pos;
    class Rules;

    class Board : public QGraphicsScene
    {
            Q_OBJECT
            Q_ENUMS ( MarkerType )

        public:

            enum MarkerType
            {
                LegalMove,
                Danger,
                Motion
            };

            Board ( KgThemeProvider* provider, QObject* parent = 0 );
            virtual ~Board();

            void populate();

            static bool isInBoard ( const Pos& pos );
            Colors playerColors() const;
            
        private:
            Rules *m_rules;
            Grid m_grid;
            QMap<Pos, Item*> m_tiles;
            QList<Item*> m_borders;
            QList<Item*> m_notations;
            Item* m_background;
            bool m_displayBorders;
            bool m_displayNotations;
            KGameRenderer* renderer;
            KgThemeProvider* m_themeProvider;

            void addPiece ( PieceType type, Color color, const Pos& pos );
            void addMarker ( const Pos& pos, MarkerType type );
            void addMarker ( const Pos& pos, const QString& spriteKey );
            void addTiles();

            Piece* pieceAt ( const QPointF& point );
            Pos mapFromScene ( const QPointF& point );
            QPointF mapToScene ( Pos pos );
            void centerOnPos ( Item* item, const Pos& pos, bool animated = true );
            void centerOnPos ( Item* item, bool animated = true );
            void removeFrame();
            void centerAndResize ( Item* item, QSize size, bool animated = true );
            PieceType getPromotedType();
            
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

        protected:
            virtual void mousePressEvent ( QGraphicsSceneMouseEvent* e );
            virtual void mouseMoveEvent( QGraphicsSceneMouseEvent* e );
            virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent* e);
            virtual void dropEvent ( QGraphicsSceneDragDropEvent* e );
            virtual void dragEnterEvent ( QGraphicsSceneDragDropEvent* e );
            virtual void dragMoveEvent ( QGraphicsSceneDragDropEvent* e );
            virtual void dragLeaveEvent ( QGraphicsSceneDragDropEvent* e );

        public slots:
            void movePiece ( const Move& move );
            void updateTheme();
            void updateGraphics();
            void changeDisplayedPlayer();
            void setCurrentColor ( Color color );
            void setPlayerColors ( Colors colors );

        signals:
            void pieceMoved ( const Move& m );
            void activePlayerChanged ( Color activePlayer );
            void displayedPlayerChanged ( Color displayedPlayer );

            void centerChanged ( const QPointF& center );
    };

}

#endif // KNIGHTS_BOARD_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
