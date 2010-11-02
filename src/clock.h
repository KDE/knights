/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2010 Thomas Kamps

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

class QTime;

namespace Knights
{

    class Clock : public QWidget
    {
            Q_OBJECT
        public:
            explicit Clock ( QWidget* parent = 0, Qt::WindowFlags f = 0 );
            virtual ~Clock();

        private:
            int hour;
            int minute;
            int second;

        public slots:
            void setTime ( int hour, int minute, int second );
            void setTime ( int seconds );
            void setTime ( const QTime& time );

        protected:
            virtual void paintEvent ( QPaintEvent* );
            virtual void resizeEvent ( QResizeEvent* );
    };

}

#endif // KNIGHTS_CLOCK_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
