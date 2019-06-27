/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>

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

#ifndef KNIGHTS_FICSPROTOCOL_H
#define KNIGHTS_FICSPROTOCOL_H

#include "proto/textprotocol.h"
#include <gamemanager.h>


class QTcpSocket;

namespace Knights {
class FicsDialog;
class ChatWidget;

typedef QPair<QString, int> FicsPlayer;

struct FicsGameOffer {
	FicsPlayer player;
	int baseTime;
	int timeIncrement;
	bool rated;
	QString variant;
	Color color;
	bool manual;
	bool formula;
	int gameId;
	QPair< int, int > ratingRange;
	bool automatic;
};

struct FicsChallenge {
	FicsPlayer player;
	int gameId;
};

enum Stage {
	ConnectStage,
	SeekStage,
	PlayStage
};

class FicsProtocol : public TextProtocol {
	Q_OBJECT
public:
	explicit FicsProtocol(QObject* parent = nullptr);
	~FicsProtocol() override;

	Features supportedFeatures() override;

	void startGame() override;
	void move ( const Move& m ) override;
	QList<ToolWidgetData> toolWidgets() override;

	void makeOffer(const Offer& offer) override;
	void acceptOffer(const Offer& offer) override;
	void declineOffer(const Offer& offer) override;

private:
	const QString movePattern;
	const QRegExp seekExp;
	const QRegExp challengeExp;
	const QRegExp moveStringExp;
	const QRegExp moveRegExp;
	const QRegExp gameStartedExp;
	const QRegExp offerExp;

	Stage m_stage;
	QString password;
	bool sendPassword;
	FicsDialog* m_widget;
	bool m_seeking;
	ChatWidget* m_chat;
	QMap<int, Offer> m_offers;
	QString otherPlayerName;

	Color parseColor( QString str );
	bool parseLine(const QString& line) override;
	bool parseStub(const QString& line) override;

public Q_SLOTS:
	void init () override;
	virtual void resign();

	void socketError();
	void dialogRejected();
	void acceptSeek ( int id );
	void acceptChallenge ( int id );
	void declineChallenge ( int id );
	void login(const QString& username, const QString& password);
	void sendChat ( QString text );

	void openGameDialog();
	void setSeeking ( bool seek );
	void setupOptions();

Q_SIGNALS:
	void sessionStarted();
	void clearSeeks();
	void gameOfferRemoved ( int id );
	void gameOfferReceived ( const FicsGameOffer& offer );
	void challengeReceived ( const FicsChallenge& challenge );
	void challengeRemoved ( int id );
};
}

#endif // KNIGHTS_FICSPROTOCOL_H
