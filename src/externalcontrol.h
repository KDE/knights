/*
 *  This file is part of Knights, a chess board for KDE SC 4.
 *  Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>
 * 
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License as
 *  published by the Free Software Foundation; either version 2 of
 *  the License or (at your option) version 3 or any later version
 *  accepted by the membership of KDE e.V. (or its successor approved
 *  by the membership of KDE e.V.), which shall act as a proxy
 *  defined in Section 14 of version 3 of the license.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KNIGHTS_EXTERNALCONTROL_H
#define KNIGHTS_EXTERNALCONTROL_H

#include <QtCore/QObject>
#include <QtCore/QPair>

namespace Knights {
  
  class Move;

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
    void slotMoveMade(const Move& move);
    
Q_SIGNALS:
    void moveMade(const QString& move);
};

}

#endif // KNIGHTS_EXTERNALCONTROL_H
