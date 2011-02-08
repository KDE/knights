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

#ifndef KNIGHTS_SEEKGRAPH_H
#define KNIGHTS_SEEKGRAPH_H

#include <KPlotWidget>
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
