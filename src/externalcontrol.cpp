/*
 *  This file is part of Knights, a chess board for KDE SC 4.
 *  SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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



