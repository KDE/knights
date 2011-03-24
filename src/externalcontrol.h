/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) 2011  Miha Čančula <miha.cancula@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KNIGHTS_EXTERNALCONTROL_H
#define KNIGHTS_EXTERNALCONTROL_H

#include <QtCore/QObject>
#include <QtCore/QPair>

namespace Knights {

class ExternalControl : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.Knights")

public:
    explicit ExternalControl(QObject* parent = 0);
    virtual ~ExternalControl();
    
public Q_SLOTS:
    void movePiece(const QString& move);
    void pauseGame();
    void resumeGame();
    void undo();
    void offerDraw();
    void adjourn();
    void abort();
    
Q_SIGNALS:
    void moveMade(const QString& move);
};

}

#endif // KNIGHTS_EXTERNALCONTROL_H
