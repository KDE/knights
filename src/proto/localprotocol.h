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

#ifndef KNIGHTS_LOCALPROTOCOL_H
#define KNIGHTS_LOCALPROTOCOL_H

#include "proto/protocol.h"


namespace Knights {

class LocalProtocol : public Protocol
{
  Q_OBJECT

public:
    LocalProtocol(QObject* parent = 0);
    virtual ~LocalProtocol();
    
    virtual Features supportedFeatures();
    virtual bool isLocal();
    
    virtual void init();
    virtual void startGame();
    virtual void move(const Move& m);
    
    virtual void makeOffer(const Offer& offer);
    virtual void acceptOffer(const Offer& offer);
    virtual void declineOffer(const Offer& offer);
};

}

#endif // KNIGHTS_LOCALPROTOCOL_H
