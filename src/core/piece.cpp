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

#include "piece.h"

#include <QtGui/QPainter>
#include <QtGui/QStyleOption>
#include <QtSvg/QSvgRenderer>

using namespace Knights;


Piece::Piece(Piece::PieceType type, Piece::Color color, QGraphicsItem* parent)
        : QGraphicsSvgItem(parent)
{
    m_color = color;
    m_type = type;
    setCacheMode(DeviceCoordinateCache);
}


Piece::~Piece()
{

}


Piece::Color Piece::oppositeColor ( Piece::Color color)
{
  switch (color)
  {
    case Black:
      return White;
    case White:
      return Black;
    default:
      return color;
  }
}


/*
there's no benefit to using KPixmapCache since QGraphicsSvgItems are cached by default

void Piece::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    QPixmap pix(option->rect.size());
    pix.fill(Qt::transparent);
    QPixmap& pixRef = pix;
    QString cacheKey = QString::fromLatin1("%1_%2_%3")
                       .arg(m_color)
                       .arg(m_type)
                       .arg(option->rect.height());
    if (!m_cache->find(cacheKey, pixRef))
    {
        QPainter painter(&pix);
        renderer()->render(&painter, elementId(), option->rect);
        m_cache->insert(cacheKey, pix);
    }
    painter->drawPixmap(option->exposedRect, pix, pix.rect());
}

*/ 


Piece::Color Piece::color()
{
    return m_color;
}

Piece::PieceType Piece::pieceType()
{
    return m_type;
}
/*
void Piece::setPixmapCache(KPixmapCache* cache)
{
    m_cache = cache;
}
*/





