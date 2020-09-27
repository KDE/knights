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

#include "externalcontrol.h"
#include "gamemanager.h"
#include "knightsadaptor.h"
#include "knightsdebug.h"

#include <QRandomGenerator>

#define FORWARD_FUNCTION(name, action) void ExternalControl::name() { Manager::self()->sendOffer(Action##action, NoColor, QRandomGenerator::global()->bounded(RAND_MAX)); }

using namespace Knights;

ExternalControl::ExternalControl(QObject* parent) : QObject(parent) {
	connect(Manager::self(), &Manager::pieceMoved, this, &ExternalControl::slotMoveMade);
	new KnightsAdaptor(this);
	qCDebug(LOG_KNIGHTS) << QDBusConnection::sessionBus().registerObject(QStringLiteral("/Knights"), this);
	qCDebug(LOG_KNIGHTS) << QDBusConnection::sessionBus().registerService(QStringLiteral("org.kde.Knights"));
}

ExternalControl::~ExternalControl() = default;

FORWARD_FUNCTION(adjourn,Adjourn)
FORWARD_FUNCTION(abort,Abort)
FORWARD_FUNCTION(pauseGame,Pause)
FORWARD_FUNCTION(resumeGame,Resume)
FORWARD_FUNCTION(undo,Undo)
FORWARD_FUNCTION(offerDraw,Draw)

void ExternalControl::movePiece(const QString& move) {
	Manager::self()->moveByExternalControl(Move(move));
}

void ExternalControl::slotMoveMade(const Knights::Move& move) {
	Q_EMIT moveMade(move.string());
}



