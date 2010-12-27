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


#include "seekgraph.h"

#include <QtGui/QPainter>
#include <KDE/KPlotObject>
#include <KPlotPoint>
#include <QtGui/QMouseEvent>
#include <KDebug>
#include "ficsprotocol.h"

using namespace Knights;

SeekGraph::SeekGraph ( QWidget* parent ) : KPlotWidget ( parent )
{
    setBackgroundColor( Qt::white );
    setShowGrid(true);
    setForegroundColor( Qt::darkGray );
    setGridColor( QColor::fromRgb(220,220,220) );
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
    if ( pointsUnderPoint( event->globalPos() ).isEmpty() )
    {
      return;
    }
    emit seekClicked( m_pointIds.value(pointsUnderPoint(event->globalPos()).first()) );
}

void SeekGraph::removeSeek ( int id )
{
    m_objects.value(id)->clearPoints();
    update();
}

void SeekGraph::addSeek ( const Knights::FicsGameOffer& offer )
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


