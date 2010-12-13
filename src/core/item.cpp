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
#include "board.h"
#include "settings.h"
#include <KDebug>

#ifdef WITH_ANIMATIONS
#  include <QtCore/QPropertyAnimation>
#  include <QtCore/QParallelAnimationGroup>
#  include <qmath.h>
#endif

using namespace Knights;

static const int fastAnimationDuration = 150;
static const int normalAnimationDuration = 250;
static const int slowAnimationDuration = 400;

#if defined WITH_KGR
Item::Item ( Renderer* renderer, const QString &key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem ) : KGameRenderedObjectItem ( renderer, key, parentItem )
{
    setBoardPos ( boardPos );
    if ( scene )
    {
        scene->addItem ( this );
    }
}

#else // WITH_KGR

#include <QtSvg/QSvgRenderer>

Item::Item ( Renderer* renderer, const QString &key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem )
        : QGraphicsSvgItem ( parentItem )
#if not defined WITH_QT_46 
        , m_rotation(0.0)
#endif
{
    setSharedRenderer ( renderer );
    setSpriteKey ( key );
    setBoardPos ( boardPos );
    setCacheMode ( DeviceCoordinateCache );
    if ( scene )
    {
        scene->addItem ( this );
    }
}

void Item::setRenderSize ( const QSize& size )
{
    resetTransform();
    QRectF normalSize = renderer()->boundsOnElement ( spriteKey() );
    qreal xScale = size.width() / normalSize.width();
    qreal yScale = size.height() / normalSize.height();
    prepareGeometryChange();
    setTransform ( QTransform().scale ( xScale, yScale ) );
#if not defined WITH_QT_46
    translate(m_origin.x(), m_origin.y());
    rotate(m_rotation);
    translate(-m_origin.x(), -m_origin.y());
#endif
}

QSize Item::renderSize() const
{
    return transform().mapRect ( boundingRect() ).size().toSize();
}

void Item::setSpriteKey ( const QString& key )
{
    setElementId ( key );
}

QString Item::spriteKey() const
{
    return elementId();
}
#endif // WITH_KGR

Item::~Item()
{
    if ( scene() )
    {
        scene()->removeItem ( this );
    }
}

Pos Item::boardPos() const
{
    return m_pos;
}

void Item::setBoardPos ( const Pos& pos )
{
    m_pos = pos;
}

void Item::move ( const QPointF& pos, qreal tileSize, bool animated )
{
#if defined WITH_ANIMATIONS
    if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant )
    {
        setPos ( pos );
    }
    else
    {
        int duration = 0;
        switch ( Settings::animationSpeed() )
        {
            case Settings::EnumAnimationSpeed::Fast:
                duration = fastAnimationDuration;
                break;
            case Settings::EnumAnimationSpeed::Normal:
                duration = normalAnimationDuration;
                break;
            case Settings::EnumAnimationSpeed::Slow:
                duration = slowAnimationDuration;
                break;
            default:
                break;
        }
        duration *= qSqrt ( QPointF ( this->pos() - pos ).manhattanLength() / tileSize );
        QPropertyAnimation* anim = new QPropertyAnimation ( this, "pos" );
        anim->setDuration ( duration );
        anim->setEasingCurve ( QEasingCurve::InOutCubic );
        anim->setEndValue ( pos );
        anim->start ( QAbstractAnimation::DeleteWhenStopped );
    }
#else
    Q_UNUSED ( animated );
    Q_UNUSED ( tileSize );
    setPos ( pos );
#endif
}

void Item::resize ( const QSize& size, bool animated )
{
#if defined WITH_ANIMATIONS
    if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant )
    {
        setRenderSize ( size );
    }
    else
    {
        int duration = 0;
        switch ( Settings::animationSpeed() )
        {
            case Settings::EnumAnimationSpeed::Fast:
                duration = fastAnimationDuration;
                break;
            case Settings::EnumAnimationSpeed::Normal:
                duration = normalAnimationDuration;
                break;
            case Settings::EnumAnimationSpeed::Slow:
                duration = slowAnimationDuration;
                break;
            default:
                break;
        }
        QPropertyAnimation* anim = new QPropertyAnimation ( this, "renderSize" );
        anim->setDuration ( duration );
        anim->setEasingCurve ( QEasingCurve::InOutCubic );
        anim->setEndValue ( size );
        anim->start ( QAbstractAnimation::DeleteWhenStopped );
    }
#else
    Q_UNUSED ( animated );
    setRenderSize ( size );
#endif
}

void Knights::Item::moveAndResize ( const QPointF& pos, qreal tileSize, const QSize& size, bool animated )
{
#if defined WITH_ANIMATIONS
    if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant )
    {
        setPos ( pos );
        setRenderSize ( size );
    }
    else
    {
        int duration = 0;
        switch ( Settings::animationSpeed() )
        {
            case Settings::EnumAnimationSpeed::Fast:
                duration = fastAnimationDuration;
                break;
            case Settings::EnumAnimationSpeed::Normal:
                duration = normalAnimationDuration;
                break;
            case Settings::EnumAnimationSpeed::Slow:
                duration = slowAnimationDuration;
                break;
            default:
                break;
        }
        duration *= qSqrt ( QPointF ( this->pos() - pos ).manhattanLength() / tileSize );
        QParallelAnimationGroup* group = new QParallelAnimationGroup;
        QPropertyAnimation* posAnimation = new QPropertyAnimation ( this, "pos" );
        posAnimation->setDuration ( duration );
        posAnimation->setEasingCurve ( QEasingCurve::InOutCubic );
        posAnimation->setEndValue ( pos );
        group->addAnimation ( posAnimation );
        QPropertyAnimation* sizeAnimation = new QPropertyAnimation ( this, "renderSize" );
        sizeAnimation->setDuration ( duration );
        sizeAnimation->setEasingCurve ( QEasingCurve::InOutCubic );
        sizeAnimation->setEndValue ( size );
        group->addAnimation ( sizeAnimation );
        group->start ( QAbstractAnimation::DeleteWhenStopped );
    }
#else
    Q_UNUSED ( animated );
    Q_UNUSED ( tileSize );
    setPos ( pos );
    setRenderSize ( size );
#endif
}

#if not defined WITH_QT_46

void Item::setRotation ( qreal angle )
{
    kDebug() << angle;
    m_rotation = angle;
}

qreal Item::rotation()
{
    return m_rotation;
}

void Item::setTransformOriginPoint(const QPointF& origin)
{
    m_origin = origin;
}

#endif



// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
