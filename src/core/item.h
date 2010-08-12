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

#include "kdeversion.h"
#if KDE_IS_VERSION(4,5,60)
    #define HAVE_RENDER
#endif

#if defined HAVE_RENDER
    #define RendererType KGameRenderer
    #define ItemBaseType KGameRenderedObjectItem
    #include <KGameRenderedObjectItem>
#else
    #define RendererType QSvgRenderer
    #define ItemBaseType QGraphicsSvgItem
    #include <QtSvg/QGraphicsSvgItem>
#endif

class Item : public ItemBaseType
{
    Q_OBJECT
    public:
        Item(RendererType* renderer, QString key, QGraphicsItem* parentItem = 0);
        virtual ~Item();
        
        #if not defined HAVE_RENDER
            // Duplicating the KGameRenderedItem API to minimize #ifdef's in Knights::Board
             void setRenderSize(QSize size);
            QSize renderSize();
            void setSpriteKey(QString key);
            QString spriteKey();
        #endif // HAVE_RENDER
};

#endif // ITEM_H
