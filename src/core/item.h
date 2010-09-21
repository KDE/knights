/*
 *  This file is part of Knights, a chess board for KDE SC 4.
 *  Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ITEM_H
#define ITEM_H

#include "renderer.h"
#include "pos.h"

#if defined WITH_KGR
    #define ItemBaseType KGameRenderedObjectItem
    #include <KGameRenderedObjectItem>
#else
    #define ItemBaseType QGraphicsSvgItem
    #include <QtSvg/QGraphicsSvgItem>
#endif

namespace Knights
{

class Item : public ItemBaseType
{
    Q_OBJECT
    Q_PROPERTY(Pos boardPos READ boardPos WRITE setBoardPos)
    
    public:
        Item(Renderer* renderer, QString key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem = 0);
        virtual ~Item();
        
        #if not defined WITH_KGR
            // Duplicating the KGameRenderedItem API to minimize #ifdef's in Knights::Board
             void setRenderSize(QSize size);
            QSize renderSize() const;
            void setSpriteKey(QString key);
            QString spriteKey() const;
        #endif // WITH_KGR
     
        void setBoardPos(const Pos& pos);
        Pos boardPos() const;
        
    private:
        Pos m_pos;
};
}
#endif // ITEM_H
