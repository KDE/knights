/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "item.h"
#include "board.h"
#include "settings.h"

#include <QPropertyAnimation>
#include <QParallelAnimationGroup>
#include <QtMath>

using namespace Knights;

static const int fastAnimationDuration = 150;
static const int normalAnimationDuration = 250;
static const int slowAnimationDuration = 400;

Item::Item ( KGameRenderer* renderer, const QString &key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem ) : KGameRenderedObjectItem ( renderer, key, parentItem ) {
	setBoardPos ( boardPos );
	if ( scene )
		scene->addItem ( this );
}

Item::~Item() {
	if ( scene() )
		scene()->removeItem ( this );
}

Pos Item::boardPos() const {
	return m_pos;
}

void Item::setBoardPos ( const Pos& pos ) {
	m_pos = pos;
}

void Item::move ( const QPointF& pos, qreal tileSize, bool animated ) {
	if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant )
		setPos ( pos );
	else {
		int duration = 0;
		switch ( Settings::animationSpeed() ) {
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

void Item::resize ( const QSize& size, bool animated ) {
	if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant )
		setRenderSize ( size );
	else {
		int duration = 0;
		switch ( Settings::animationSpeed() ) {
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

void Item::moveAndResize ( const QPointF& pos, qreal tileSize, const QSize& size, bool animated ) {
	if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant ) {
		setPos ( pos );
		setRenderSize ( size );
	} else {
		int duration = 0;
		switch ( Settings::animationSpeed() ) {
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
