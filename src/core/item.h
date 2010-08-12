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
