/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2010, 2011 Miha Čančula <miha@noughmad.eu>

    Plasma analog-clock drawing code:
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clock.h"

#include <math.h>

#include <KSvg/Svg>

#include <QPainter>
#include <QPaintEvent>

using namespace Knights;

Clock::Clock(QWidget* parent) : QWidget(parent),
	m_theme(new KSvg::Svg(this)),
	m_repaintCache(RepaintNone),
	m_verticalTranslation(0.) {

    m_theme->imageSet()->setBasePath(QStringLiteral("plasma/desktoptheme"));
	m_theme->setImagePath(QStringLiteral("widgets/clock") );
	m_theme->setContainsMultipleImages(true);
}

Clock::~Clock() {
	delete m_theme;
}

void Clock::showEvent( QShowEvent *event ) {
	setClockSize( size() );
	QWidget::showEvent( event );
}

void Clock::resizeEvent( QResizeEvent * ) {
	setClockSize( size() );
}

void Clock::setClockSize(const QSize &size) {
	int dim = qMin(size.width(), size.height());
	QSize newSize = QSize(dim, dim);

	if (newSize != m_faceCache.size()) {
		m_faceCache = QPixmap(newSize);
		m_handsCache = QPixmap(newSize);
		m_glassCache = QPixmap(newSize);

		m_theme->resize(newSize);
		m_repaintCache = RepaintAll;
	}
}

void Clock::setTime(const QTime &time) {
	if (m_repaintCache == RepaintNone)
		m_repaintCache = RepaintHands;
	this->time = time;
	update();
}

void Clock::setTime(int miliSeconds) {
	setTime(QTime().addMSecs(miliSeconds));
}

void Clock::drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName) {
	// this code assumes the following conventions in the svg file:
	// - the _vertical_ position of the hands should be set with respect to the center of the face
	// - the _horizontal_ position of the hands does not matter
	// - the _shadow_ elements should have the same vertical position as their _hand_ element counterpart

	QRectF elementRect;
	QString name = handName + QLatin1String( "HandShadow" );
	if (m_theme->hasElement(name)) {
		p->save();

		elementRect = m_theme->elementRect(name);
		if( rect.height() < 64 )
			elementRect.setWidth( elementRect.width() * 2.5 );
		static const QPoint offset = QPoint(2, 3);

		p->translate(rect.x() + (rect.width() / 2) + offset.x(), rect.y() + (rect.height() / 2) + offset.y());
		p->rotate(rotation);
		p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
		m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

		p->restore();
	}

	p->save();

	name = handName + QLatin1String("Hand");
	elementRect = m_theme->elementRect(name);
	if (rect.height() < 64)
		elementRect.setWidth(elementRect.width() * 2.5);

	p->translate(rect.x() + rect.width()/2, rect.y() + rect.height()/2);
	p->rotate(rotation);
	p->translate(-elementRect.width()/2, elementRect.y()-verticalTranslation);
	m_theme->paint(p, QRectF(QPointF(0, 0), elementRect.size()), name);

	p->restore();
}

void Clock::paintInterface(QPainter *p, const QRect &rect) {
	const bool m_showSecondHand = true;

	// compute hand angles
	// Because this clock shows time remainig, all the angles are negative
	const qreal minutes = -6.0 * time.minute() - 0.1 * time.second() - 180;
	const qreal hours = -30.0 * time.hour() - 0.5 * time.minute() - 180;
	qreal seconds = 0;
	if (m_showSecondHand) {
		static const double anglePerSec = 6;
		seconds = -anglePerSec * ( time.second() + time.msec() / 1000.0 ) - 180;
	}

	// paint face and glass cache
	QRect faceRect = m_faceCache.rect();
	if (m_repaintCache == RepaintAll) {
		m_faceCache.fill(Qt::transparent);
		m_glassCache.fill(Qt::transparent);

		QPainter facePainter(&m_faceCache);
		QPainter glassPainter(&m_glassCache);
		facePainter.setRenderHint(QPainter::SmoothPixmapTransform);
		glassPainter.setRenderHint(QPainter::SmoothPixmapTransform);

		m_theme->paint(&facePainter, m_faceCache.rect(), QStringLiteral("ClockFace") );

		glassPainter.save();
		QRectF elementRect = QRectF(QPointF(0, 0), m_theme->elementSize(QStringLiteral("HandCenterScrew")));
		glassPainter.translate(faceRect.width() / 2 - elementRect.width() / 2, faceRect.height() / 2 - elementRect.height() / 2);
		m_theme->paint(&glassPainter, elementRect, QStringLiteral("HandCenterScrew"));
		glassPainter.restore();

		m_theme->paint(&glassPainter, faceRect, QStringLiteral("Glass"));

		// get vertical translation, see drawHand() for more details
		m_verticalTranslation = m_theme->elementRect(QStringLiteral("ClockFace")).center().y();
	}

	// paint hour and minute hands cache
	if (m_repaintCache == RepaintHands || m_repaintCache == RepaintAll) {
		m_handsCache.fill(Qt::transparent);

		QPainter handsPainter(&m_handsCache);
		handsPainter.drawPixmap(faceRect, m_faceCache, faceRect);
		handsPainter.setRenderHint(QPainter::SmoothPixmapTransform);

		drawHand(&handsPainter, faceRect, m_verticalTranslation, hours, QStringLiteral("Hour"));
		drawHand(&handsPainter, faceRect, m_verticalTranslation, minutes, QStringLiteral("Minute"));
	}

	// reset repaint cache flag
	m_repaintCache = RepaintNone;

	// paint caches and second hand
	QRect targetRect = faceRect;
	if (targetRect.width() < rect.width())
		targetRect.moveLeft((rect.width() - targetRect.width()) / 2);
	if (targetRect.height() < rect.height())
		targetRect.moveTop((rect.height() - targetRect.height()) / 2);

	p->drawPixmap(targetRect, m_handsCache, faceRect);
	if (m_showSecondHand) {
		p->setRenderHint(QPainter::SmoothPixmapTransform);
		drawHand(p, targetRect, m_verticalTranslation, seconds, QStringLiteral("Second"));
	}
	p->drawPixmap(targetRect, m_glassCache, faceRect);
}

void Clock::paintEvent( QPaintEvent * ) {
	QPainter paint(this);

	paint.setRenderHint(QPainter::Antialiasing);
	paintInterface(&paint, rect());
}

#include "moc_clock.cpp"
