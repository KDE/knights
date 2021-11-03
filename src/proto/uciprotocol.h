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
