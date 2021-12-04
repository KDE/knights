/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_UCIPROTOCOL_H
#define KNIGHTS_UCIPROTOCOL_H

#include "proto/computerprotocol.h"
#include <QStack>


namespace Knights {

class UciProtocol : public ComputerProtocol {

public:
	explicit UciProtocol(QObject* parent = nullptr);
	~UciProtocol() override;

	Features supportedFeatures() override;
	void declineOffer(const Knights::Offer& offer) override;
	void acceptOffer(const Knights::Offer& offer) override;
	void makeOffer(const Knights::Offer& offer) override;
	void startGame() override;
	void init() override;
	void move(const Knights::Move& m) override;
	void setDifficulty(int depth, int memory) override;

private Q_SLOTS:
	void changeCurrentTime(Knights::Color color, const QTime& time);
	void requestNextMove();

protected:
	bool parseStub(const QString& line) override;
	bool parseLine(const QString& line) override;

private:
	QStack<Move> mMoveHistory;
	int mWhiteTime;
	int mBlackTime;
	Move mPonderMove;
	int mDifficulty;
};

}

#endif // KNIGHTS_UCIPROTOCOL_H
