/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_PROTOCOL_H
#define KNIGHTS_PROTOCOL_H

#include <core/move.h>
#include <core/piece.h>

#include <QObject>
#include <QTime>
#include <QPointer>

namespace Knights {

class Offer;


struct TimeControl;


class ChatWidget;

class ProtocolPrivate;

class Protocol : public QObject {
	Q_OBJECT
	Q_ENUMS ( Feature )
	Q_ENUMS ( ErrorCode )
	Q_FLAGS ( Features )
	Q_PROPERTY ( Color color READ color WRITE setColor )
	Q_PROPERTY ( QString playerName READ playerName WRITE setPlayerName )

public:
	enum Feature {
		NoFeatures = 0x00, /**< The protocol supports none of the optional features */
		TimeLimit = 0x01, /**< The protocol supports setting a time limit for players */
		SetTimeLimit = 0x02, /**< The protocol can enable/disable and set the time limits */
		UpdateTime = 0x04, /**< The protocol notifies the programs of changes to times */
		Pause = 0x08, /**< The protocol supports pausing the clock */
		History = 0x10,
		Undo = 0x20, /**< It is possible to undo a move */
		GameOver = 0x40, /**< The protocol emits gameOver() when the game is over */
		Draw = 0x80,
		Adjourn = 0x100,
		Resign = 0x200,
		Abort = 0x400,
		SetDifficulty = 0x800, /**< It is possible to set the difficulty level before starting the game */
		AdjustDifficulty = 0x1000 /**< It is possible to change the difficulty level during the game */
	};
	Q_DECLARE_FLAGS ( Features, Feature )

	enum ErrorCode {
		NoError = 0,
		UserCancelled,
		NetworkError,
		InstallationError,
		UnknownError
	};

	enum ToolWidgetType {
		ConsoleToolWidget,
		ChatToolWidget,
		OtherToolWidget
	};

	struct ToolWidgetData {
		QWidget* widget;
		QString title;
		QString name;
		ToolWidgetType type;
		Color owner;
	};

	static QString stringFromErrorCode ( ErrorCode code );
	static Protocol* white();
	static void setWhiteProtocol ( Protocol* p );
	static Protocol* black();
	static void setBlackProtocol ( Protocol* p );
	static Protocol* byColor ( Color color );

	explicit Protocol(QObject* parent = nullptr);
	~Protocol() override;

	// Needed functions
	virtual bool isLocal();
	virtual bool isComputer();

	Color color() const;
	QString playerName() const;
	QVariant attribute ( const QString& attribute ) const;
	QVariant attribute ( const char* attribute ) const;

	void setColor ( Color color );
	void setPlayerName ( const QString& name );
	void setAttribute ( const QString& attribute, QVariant value );
	void setAttribute ( const char* attribute, QVariant value );
	void setAttributes ( QVariantMap attributes );

protected:

	ChatWidget* createChatWidget();
	ChatWidget* createConsoleWidget();
	void initComplete();

public Q_SLOTS:
	virtual void move ( const Move& m ) = 0;
	virtual void init() = 0;
	virtual void startGame() = 0;
	virtual void makeOffer ( const Offer& offer ) = 0;
	virtual void acceptOffer ( const Offer& offer ) = 0;
	virtual void declineOffer ( const Offer& offer ) = 0;

	// Optional features
	virtual void setWinner ( Color winner );

public:
	virtual Features supportedFeatures();
	virtual int timeRemaining();
	virtual QList<ToolWidgetData> toolWidgets();
	virtual void setTimeControl ( const TimeControl& c );
	virtual bool isReady();
	virtual void setDifficulty ( int depth, int memory );

Q_SIGNALS:
	void pieceMoved ( const Move& m );
	void illegalMove();
	void gameOver ( Color winner );


	void initSuccesful();
	void error ( const Protocol::ErrorCode& errorCode, const QString& errorString = QString() );

	void timeChanged ( const QTime& time );
	void undoPossible ( bool possible );
	void redoPossible ( bool possible );

private:
	static QPointer<Protocol> m_white;
	static QPointer<Protocol> m_black;
	ProtocolPrivate* d_ptr;
	Q_DECLARE_PRIVATE ( Protocol )
};

Q_DECLARE_OPERATORS_FOR_FLAGS ( Protocol::Features )
}

#endif // KNIGHTS_PROTOCOL_H
