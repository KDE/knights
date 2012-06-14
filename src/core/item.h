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

#ifndef KNIGHTS_ITEM_H
#define KNIGHTS_ITEM_H

#include "pos.h"

#include <KGameRenderedObjectItem>

namespace Knights
{

    class Item : public KGameRenderedObjectItem
    {
            Q_OBJECT
            Q_PROPERTY ( Pos boardPos READ boardPos WRITE setBoardPos )
            Q_PROPERTY ( QSize renderSize READ renderSize WRITE setRenderSize )

        public:
            Item ( KGameRenderer* renderer, const QString& key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem = 0 );
            virtual ~Item();

            void setBoardPos ( const Pos& pos );
            Pos boardPos() const;

            void move ( const QPointF& pos, qreal tileSize, bool animated = true );
            void resize ( const QSize& size, bool animated = true );
            void moveAndResize ( const QPointF& pos, qreal tileSize, const QSize& size, bool animated = true );

        private:
            Pos m_pos;

    };
}
#endif // KNIGHTS_ITEM_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
