/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_POS_H
#define KNIGHTS_POS_H

#include <QPair>
#include <QChar>
namespace Knights {

class Pos : public QPair<int, int> {
public:
	Pos();
	Pos ( const int& t1, const int& t2 );
    explicit Pos (const QString &);
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
