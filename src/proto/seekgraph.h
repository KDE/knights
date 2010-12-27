/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef KNIGHTS_SEEKGRAPH_H
#define KNIGHTS_SEEKGRAPH_H

#include <kplotwidget.h>

#include <QtCore/QMap>


namespace Knights
{
  struct FicsGameOffer;

    class SeekGraph : public KPlotWidget
    {
	Q_OBJECT
        public:
            explicit SeekGraph ( QWidget* parent = 0 );
            virtual ~SeekGraph();

	    void addSeek ( const FicsGameOffer& offer );
	    void removeSeek ( int id );
            void clearOffers();
	    void setRect ( const QRectF rect );

        protected:
            virtual void paintEvent ( QPaintEvent* event );
            virtual void mouseMoveEvent ( QMouseEvent* event );
            virtual void mouseReleaseEvent ( QMouseEvent* event );

	signals:
	    void seekClicked( int id );

    private:
      QMap<KPlotPoint*, int> m_pointIds;
      QMap<int, KPlotObject*> m_objects;
      QRectF m_dataRect;
    };

}

#endif // KNIGHTS_SEEKGRAPH_H
