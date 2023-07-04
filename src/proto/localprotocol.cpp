/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "localprotocol.h"
#include "gamemanager.h"

#include <KUser>

using namespace Knights;

void LocalProtocol::init (  ) {
	KUser user;
	if ( user.isValid() ) {
		QVariant fullName = user.property(KUser::FullName);
		setPlayerName ( ( QVariant() == fullName ) ? user.loginName() : fullName.toString() );
	} else
		setPlayerName ( colorName ( color() ) );
	initComplete();
}

void LocalProtocol::startGame() {

}

void LocalProtocol::move ( const Move& m ) {
	Q_UNUSED(m)
}

LocalProtocol::LocalProtocol ( QObject* parent ) : Protocol ( parent ) {
}

LocalProtocol::~LocalProtocol() = default;

bool LocalProtocol::isLocal() {
	return true;
}

Protocol::Features LocalProtocol::supportedFeatures() {
	return Pause | Undo | TimeLimit;
}

void LocalProtocol::makeOffer(const Offer& offer) {
	offer.accept();
}

void LocalProtocol::acceptOffer(const Offer& offer) {
	Q_UNUSED(offer);
}

void LocalProtocol::declineOffer(const Offer& offer) {
	Q_UNUSED(offer);
}

#include "moc_localprotocol.cpp"
