#include "item.h"

#if defined HAVE_RENDER
Item::Item(KGameRenderer* renderer, QString key, QGraphicsItem* parentItem): KGameRenderedObjectItem(renderer, key, parentItem)
{

}

Item::~Item()
{

}

#else

#include <QtSvg/QSvgRenderer>

Item::Item(QSvgRenderer* renderer, QString key, QGraphicsItem* parentItem)
    : QGraphicsSvgItem( parentItem)
{
    setSharedRenderer(renderer);
    setElementId(key);
}

Item::~Item()
{

}

void Item::setRenderSize(QSize size)
{
    QRectF normalSize = renderer()->boundsOnElement(spriteKey());
    qreal xScale = size.width() / normalSize.width();
    qreal yScale = size.height() / normalSize.height();
    setScale(qMin(xScale, yScale));
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



