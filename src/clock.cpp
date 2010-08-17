/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2010  Thomas Kamps

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

#include "clock.h"

#include <math.h>
#include <QPainter>
#include <QPaintEvent>
#include <QTime>

using namespace Knights;

Clock::Clock(QWidget *parent, Qt::WindowFlags f) : QWidget(parent)
{
    Q_UNUSED(f);
    hour = 0;
    minute = 0;
    second = 0;
}

Clock::~Clock()
{

}


void Clock::setTime(int hour, int minute, int second) 
{
    if (hour < 0) return;
    if (hour > 11) return;
    if (minute < 0) return;
    if (minute > 59) return;
    if (second < 0) return;
    if (second > 59) return;
    this->hour = hour;
    this->minute = minute;
    this->second = second;
    update();
}

void Clock::setTime(int seconds) 
{
    if (seconds < 0) return;
    if (seconds > 86399) return;

    int secs = seconds;
    int h = secs/3600;
    secs = secs%3600;
    int m = secs/60;
    int s = secs%60;

    setTime(h,m,s);
}

void Clock::setTime(const QTime& time)
{
    setTime(time.hour(), time.minute(), time.second());
}

void Clock::resizeEvent(QResizeEvent* e) 
{
    update();
    QWidget::resizeEvent(e);
}

void Clock::paintEvent(QPaintEvent* e) 
{
    qreal PI = 3.141592653;
    int size = width();
    if (height() < size) size = height();

    QPainter p(this);

    //Draw border of clock
    p.setPen(QPen(QColor(0,0,0), 3));
    p.drawEllipse((width()-size)/2, (height()-size)/2, size-2, size-2);

    //Draw hour-pointer
    qreal angle = PI/2 - hour*(PI/6);
    int endX = width()/2+cos(angle)*size/4;
    int endY = height()/2-sin(angle)*size/4;
    p.drawLine(width()/2, height()/2, endX, endY);

    //Draw minute pointer
    p.setPen(QPen(QColor(0,0,0), 2));
    angle = PI/2 - minute*(PI/30);
    endX = width()/2+cos(angle)*2*size/6;
    endY = height()/2-sin(angle)*2*size/6;
    p.drawLine(width()/2, height()/2, endX, endY);

    //Draw second pointer
    p.setPen(QPen(QColor(0,0,0), 1));
    angle = PI/2 - second*(PI/30);
    endX = width()/2+cos(angle)*size/2;
    endY = height()/2-sin(angle)*size/2;
    p.drawLine(width()/2, height()/2, endX, endY);

    e->accept();
}
