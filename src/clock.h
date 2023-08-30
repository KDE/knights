/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2010, 2011 Miha Čančula <miha@noughmad.eu>

    Plasma analog-clock drawing code:
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2007 Riccardo Iaconelli <riccardo@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_CLOCK_H
#define KNIGHTS_CLOCK_H

#include <QWidget>
#include <QTime>

#if QT_VERSION_MAJOR == 5
namespace Plasma {
class Svg;
}
#else
namespace KSvg {
class Svg;
}
#endif

namespace Knights {
/**
 * This class is copied from the Date and Time KCM.
 * See https://invent.kde.org/plasma/plasma-desktop/-/blob/master/kcms/dateandtime/dtime.h
 *
 * Note that this is a chess clock, which is set to negative time before a match, so all angles are reversed.
 * See paintInterface for details.
 */
class Clock : public QWidget {
	Q_OBJECT

public:
	explicit Clock( QWidget* parent = nullptr);
	~Clock() override;

	void setTime(const QTime&);
	void setTime(int miliSeconds);

protected:
	void paintEvent( QPaintEvent *event ) override;
	void showEvent( QShowEvent *event ) override;
	void resizeEvent( QResizeEvent *event ) override;

private:
	void setClockSize(const QSize &size);
	void drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName);
	void paintInterface(QPainter *p, const QRect &rect);

	QTime time;
#if QT_VERSION_MAJOR == 5
	Plasma::Svg *m_theme;
#else
	KSvg::Svg *m_theme;
#endif
	enum RepaintCache {
		RepaintNone,
		RepaintAll,
		RepaintHands
	};
	RepaintCache m_repaintCache;
	QPixmap m_faceCache;
	QPixmap m_handsCache;
	QPixmap m_glassCache;
	qreal m_verticalTranslation;
};

}

#endif // KNIGHTS_CLOCK_H
