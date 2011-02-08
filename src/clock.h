/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2010,2011 Miha Čančula <miha@noughmad.eu>

    Plasma analog-clock drawing code:
    Copyright 2007 by Aaron Seigo <aseigo@kde.org>
    Copyright 2007 by Riccardo Iaconelli <riccardo@kde.org>

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

#ifndef KNIGHTS_CLOCK_H
#define KNIGHTS_CLOCK_H

#include <QtGui/QWidget>
#include <QtCore/QTime>

namespace Plasma
{
class Svg;
}

namespace Knights
{
/**
 * This class is copied from the Date and Time KCM.
 * See http://websvn.kde.org/trunk/KDE/kdebase/workspace/kcontrol/dateandtime/dtime.h?&view=markup
 *
 * Note that this is a chess clock, which is set to negative time before a match, so all angles are reversed.
 * See paintInterface for details.
 */
class Clock : public QWidget
{
    Q_OBJECT

public:
    Clock( QWidget *parent=0 );
    ~Clock();

    void setTime(const QTime&);
    void setTime(int miliSeconds);

protected:
    virtual void paintEvent( QPaintEvent *event );
    virtual void showEvent( QShowEvent *event );
    virtual void resizeEvent( QResizeEvent *event );

private:
    void setClockSize(const QSize &size);
    void drawHand(QPainter *p, const QRect &rect, const qreal verticalTranslation, const qreal rotation, const QString &handName);
    void paintInterface(QPainter *p, const QRect &rect);

private:
    QTime time;
    Plasma::Svg *m_theme;
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
// kate: indent-mode cstyle; space-indent on; indent-width 0;  replace-tabs on;
