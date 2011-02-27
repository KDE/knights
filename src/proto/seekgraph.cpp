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

#include "proto/seekgraph.h"
#include "proto/ficsprotocol.h"

#include <KPlotObject>
#include <KPlotPoint>
#include <KPlotAxis>
#include <KDebug>
#include <KLocale>

#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

using namespace Knights;

SeekGraph::SeekGraph ( QWidget* parent ) : KPlotWidget ( parent )
{
    setBackgroundColor( Qt::white );
    setShowGrid(true);
    setForegroundColor( Qt::black );
    setGridColor( QColor::fromRgb(220,220,220) );
    axis ( BottomAxis )->setLabel ( i18n("Time limit [minutes]") );
    axis ( LeftAxis )->setLabel ( i18n("Opponent's rating") );
    setMouseTracking(true);
}

SeekGraph::~SeekGraph()
{

}

void SeekGraph::paintEvent ( QPaintEvent* event )
{
    QFrame::paintEvent( event );
    QPainter p;

     p.begin( this );
     p.setRenderHint( QPainter::Antialiasing, antialiasing() );
     p.fillRect( rect(), backgroundColor() );
     p.translate( leftPadding() + 0.5, topPadding() + 0.5 );

     p.setClipping( false );
     drawAxes( &p );

     setPixRect();
     p.setClipRect( pixRect() );
     p.setClipping( true );

     resetPlotMask();

     foreach( KPlotObject *po, plotObjects() )
     {
       po->draw( &p, this );
     }

     p.end();
}

void SeekGraph::mouseMoveEvent ( QMouseEvent* event )
{
    bool isOverPoint = false;
    foreach ( KPlotObject* o, plotObjects() )
    {
      if ( o->points().isEmpty() )
      {
	continue;
      }
      QPointF d = mapToWidget ( o->points().first()->position()) - event->pos() + QPoint(leftPadding(), topPadding());
      if ( d.x() * d.x() + d.y() * d.y() < 64.0 )
      {
	isOverPoint = true;
	break;
      }
    }
    if ( isOverPoint )
    {
      setCursor( Qt::PointingHandCursor );
      event->accept();
    }
    else
    {
      setCursor( Qt::ArrowCursor );
      event->ignore();
    }
}

void SeekGraph::mouseReleaseEvent ( QMouseEvent* event )
{
    foreach ( KPlotObject* o, plotObjects() )
    {
      if ( o->points().isEmpty() )
      {
	continue;
      }
      QPointF d = mapToWidget ( o->points().first()->position()) - event->pos() + QPoint(leftPadding(), topPadding());
      if ( d.x() * d.x() + d.y() * d.y() < 64.0 )
      {
	emit seekClicked ( m_objects.key(o) );
	return;
      }
    }
}

void SeekGraph::removeSeek ( int id )
{
    m_objects.value(id)->clearPoints();
    update();
}

void SeekGraph::addSeek ( const FicsGameOffer& offer )
{
    KPlotObject* object = new KPlotObject;
    QPen labelPen = object->labelPen();
    labelPen.setColor(Qt::black);
    object->setLabelPen(labelPen);
    QPointF point = QPointF( offer.baseTime, offer.player.second );
    object->addPoint( point, QLatin1Char(' ') + offer.player.first + QLatin1Char(' ') );
    object->setBrush( QBrush(Qt::blue) );
    object->setSize(8.0);
    addPlotObject(object);
    QRectF pRect = QRectF( point - QPointF(2,50), QSizeF(5,100) );
    setRect ( m_dataRect.isValid() ? m_dataRect.united(pRect) : pRect );
    m_objects.insert ( offer.gameId, object );
    m_pointIds.insert ( object->points().first(), offer.gameId );
    update();
}

void SeekGraph::clearOffers()
{
  m_objects.clear();
  m_pointIds.clear();
  removeAllPlotObjects();
  update();
}

void SeekGraph::setRect ( const QRectF rect )
{
    m_dataRect = rect;
    setLimits ( rect.left(), rect.right(), rect.top(), rect.bottom() );
}


