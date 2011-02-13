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

#include "pos.h"
#include "board.h"

#include <QtCore/QDebug>

namespace Knights
{
    const QString Pos::rowNames = QString::fromAscii ( "abcdefgh" );


    QChar Pos::row ( int num )
    {
        if ( num > 0 && num < 9 )
        {
            return rowNames[num - 1];
        }
        else
        {
            return QLatin1Char ( 'r' );
        }
    }

    int Pos::numFromRow ( const QChar& row )
    {
        return rowNames.indexOf ( row ) + 1;
    }

    Pos::Pos()
    {

    }

    Pos::Pos ( const int& t1, const int& t2 ) : QPair< int, int > ( t1, t2 )
    {

    }

    Pos::Pos ( const QString string )
    {
        if ( string.size() == 2 )
        {
            first = numFromRow ( string.at ( 0 ) );
            second = string.right ( 1 ).toInt();
        }
    }

    Pos::~Pos()
    {

    }

    QString Pos::string() const
    {
        if ( isValid() )
        {
            return row ( first ) + QString::number ( second );
        }
        else
        {
            return QString();
        }
    }
    
    bool Pos::isValid() const
    {
        return first > 0 && first < 9 && second > 0 && second < 9;
    }

    const Pos& Pos::operator+= ( const Pos & other )
    {
        first += other.first;
        second += other.second;
        return *this;
    }

    Pos operator+ ( const Pos& one, const Pos& other )
    {
        return Pos ( one.first + other.first, one.second + other.second );
    }

    Pos operator- ( const Pos& one, const Pos& other )
    {
        return Pos ( one.first - other.first, one.second - other.second );
    }

    Pos operator* ( int m, const Pos& other )
    {
        return Pos ( m*other.first, m*other.second );
    }

    Pos operator/ ( const Pos& other, int m )
    {
        return Pos ( other.first / m, other.second / m );
    }
}

    QDebug operator<< ( QDebug debug, const Knights::Pos& pos )
    {
        debug.nospace() << Knights::Pos::row ( pos.first ) << pos.second;
        return debug;
    }

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
