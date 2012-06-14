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

#include "item.h"
#include "board.h"
#include "settings.h"

#include <KDebug>

#include <QtCore/QPropertyAnimation>
#include <QtCore/QParallelAnimationGroup>
#include <qmath.h>

using namespace Knights;

static const int fastAnimationDuration = 150;
static const int normalAnimationDuration = 250;
static const int slowAnimationDuration = 400;

Item::Item ( KGameRenderer* renderer, const QString &key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem ) : KGameRenderedObjectItem ( renderer, key, parentItem )
{
    setBoardPos ( boardPos );
    if ( scene )
    {
        scene->addItem ( this );
    }
}

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
}

void Item::resize ( const QSize& size, bool animated )
{
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
}

void Item::moveAndResize ( const QPointF& pos, qreal tileSize, const QSize& size, bool animated )
{
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
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
