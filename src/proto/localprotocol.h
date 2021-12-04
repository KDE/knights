/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_LOCALPROTOCOL_H
#define KNIGHTS_LOCALPROTOCOL_H

#include "proto/protocol.h"


namespace Knights {

class LocalProtocol : public Protocol {
	Q_OBJECT

public:
	explicit LocalProtocol(QObject* parent = nullptr);
	~LocalProtocol() override;

	Features supportedFeatures() override;
	bool isLocal() override;

	void init() override;
	void startGame() override;
	void move(const Move& m) override;

	void makeOffer(const Offer& offer) override;
	void acceptOffer(const Offer& offer) override;
	void declineOffer(const Offer& offer) override;
};

}

#endif // KNIGHTS_LOCALPROTOCOL_H
