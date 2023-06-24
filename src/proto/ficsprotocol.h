/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009-2010 Miha Čančula <miha.cancula@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_FICSPROTOCOL_H
#define KNIGHTS_FICSPROTOCOL_H

#include "proto/textprotocol.h"
#include <gamemanager.h>

#include <QRegExp>

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
