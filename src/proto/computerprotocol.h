/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2011  Miha Čančula <miha@noughmad.eu>

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

#ifndef KNIGHTS_COMPUTERPROTOCOL_H
#define KNIGHTS_COMPUTERPROTOCOL_H

#include <proto/textprotocol.h>

class KProcess;

namespace Knights {

class ComputerProtocol : public TextProtocol
{
public:
    ComputerProtocol(QObject* parent = 0);
    virtual ~ComputerProtocol();

    virtual void startProgram();
    virtual bool isComputer();
    virtual QList< ToolWidgetData > toolWidgets();
    
protected:
    KProcess* mProcess;
};

}

#endif // KNIGHTS_COMPUTERPROTOCOL_H
