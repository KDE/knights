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

#ifndef KNIGHTS_POS_H
#define KNIGHTS_POS_H

#include <QtCore/QPair>
#include <QtCore/QDataStream>

namespace Knights
{

    class Pos : public QPair<int, int>
    {
        public:
            Pos();
            Pos ( const int& t1, const int& t2 );
            Pos ( const QString string );
            ~Pos();

            static QChar row ( int num );
            static int numFromRow ( const QChar& row );

            QString string() const;
            bool isValid() const;

            const Pos& operator+= ( const Pos& other );

        private:
            static const QString rowNames;
    };

    Pos operator+ ( const Pos& one, const Pos& other );
    Pos operator- ( const Pos& one, const Pos& other );
    Pos operator* ( int m, const Pos& other );
    Pos operator/ ( const Pos& other, int m );
}
    QDebug operator<< ( QDebug debug, const Knights::Pos &pos );
    
#endif // KNIGHTS_POS_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
