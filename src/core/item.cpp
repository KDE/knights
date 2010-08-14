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

#include "item.h"
#include <QGraphicsScene>
#include <KDebug>

#if defined HAVE_RENDER
Item::Item(KGameRenderer* renderer, QString key, QGraphicsScene* scene, QGraphicsItem* parentItem): KGameRenderedObjectItem(renderer, key, parentItem)
{
    if (scene)
    {
        scene->addItem ( this );
}

Item::~Item()
{

}

#else

#include <QtSvg/QSvgRenderer>

Item::Item(RendererType* renderer, QString key, QGraphicsScene* scene, QGraphicsItem* parentItem)
    : QGraphicsSvgItem( parentItem)
{
    setSharedRenderer(renderer);
    setSpriteKey(key);
    if (scene)
    {
        scene->addItem ( this );
    }
}

Item::~Item()
{

}

void Item::setRenderSize(QSize size)
{
    resetTransform();
    QRectF normalSize = renderer()->boundsOnElement(spriteKey());
    kDebug() << normalSize;
    qreal xScale = size.width() / normalSize.width();
    qreal yScale = size.height() / normalSize.height();
    setTransform(QTransform().scale( xScale, yScale ));
    update();
}

QSize Item::renderSize()
{
    return boundingRect().size().toSize();
}

void Item::setSpriteKey(QString key)
{
    setElementId(key);
}

QString Item::spriteKey()
{
    return elementId();
}
#endif



