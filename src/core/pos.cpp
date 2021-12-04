/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pos.h"
#include "board.h"

#include <QDebug>

namespace Knights {
const QString Pos::rowNames = QStringLiteral ( "abcdefgh" );


QChar Pos::row ( int num ) {
	if ( num > 0 && num < 9 )
		return rowNames[num - 1];
	else
		return QLatin1Char ( 'r' );
}

int Pos::numFromRow ( const QChar& row ) {
	return rowNames.indexOf ( row ) + 1;
}

Pos::Pos() {

}

Pos::Pos ( const int& t1, const int& t2 ) : QPair< int, int > ( t1, t2 ) {

}

Pos::Pos ( const QString &string ) {
	if ( string.size() == 2 ) {
		first = numFromRow ( string.at ( 0 ) );
		second = string.right ( 1 ).toInt();
	}
}

Pos::~Pos() = default;

QString Pos::string() const {
	if ( isValid() )
		return row ( first ) + QString::number ( second );
	else
		return QString();
}

bool Pos::isValid() const {
	return first > 0 && first < 9 && second > 0 && second < 9;
}

const Pos& Pos::operator+= ( const Pos & other ) {
	first += other.first;
	second += other.second;
	return *this;
}

Pos operator+ ( const Pos& one, const Pos& other ) {
	return Pos ( one.first + other.first, one.second + other.second );
}

Pos operator- ( const Pos& one, const Pos& other ) {
	return Pos ( one.first - other.first, one.second - other.second );
}

Pos operator* ( int m, const Pos& other ) {
	return Pos ( m*other.first, m*other.second );
}

Pos operator/ ( const Pos& other, int m ) {
	return Pos ( other.first / m, other.second / m );
}
}

QDebug operator<< ( QDebug debug, const Knights::Pos& pos ) {
	debug.nospace() << Knights::Pos::row ( pos.first ) << pos.second;
	return debug;
}
