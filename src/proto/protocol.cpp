/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "proto/protocol.h"
#include "proto/chatwidget.h"
#include "core/move.h"
#include <gamemanager.h>

#include <KLocalizedString>


namespace Knights {
int id = qRegisterMetaType<Protocol::ErrorCode> ( "Protocol::ErrorCode" );

QPointer<Protocol> Protocol::m_white = nullptr;
QPointer<Protocol> Protocol::m_black = nullptr;

class ProtocolPrivate {
public:

	ProtocolPrivate();

	QVariantMap attributes;
	Protocol* white;
	Protocol* black;
	Color color;
	bool ready;
	int nextId;
};

ProtocolPrivate::ProtocolPrivate()
	: white(nullptr),
	black(nullptr),
	color(NoColor),
	ready(false),
	nextId(0) {

}

Protocol::Protocol ( QObject* parent ) : QObject ( parent ), d_ptr ( new ProtocolPrivate ) {
}

Protocol::~Protocol() {
	delete d_ptr;
}

QString Protocol::stringFromErrorCode ( Protocol::ErrorCode code ) {
	switch ( code ) {
	case NoError:
		return i18n ( "No Error" );

	case UserCancelled:
		return i18n ( "User Canceled" );

	case NetworkError:
		return i18n ( "Network Error" );

	case UnknownError:
		return i18n ( "Unknown Error" );

	case InstallationError:
		return i18n ( "Program Error" );

	default:
		return QString();
	}
}

void Protocol::setWhiteProtocol(Protocol* p) {
	p->setColor(White);
	m_white = p;
}

void Protocol::setBlackProtocol(Protocol* p) {
	p->setColor(Black);
	m_black = p;
}

Protocol* Protocol::white() {
	return m_white;
}

Protocol* Protocol::black() {
	return m_black;
}

Protocol* Protocol::byColor(Color color) {
	switch ( color ) {
	case White:
		return white();
	case Black:
		return black();
	case NoColor:
		return nullptr;
	}
	return nullptr;
}
void Protocol::setColor ( Color color ) {
	Q_D(Protocol);
	d->color = color;
}

Color Protocol::color() const {
	Q_D(const Protocol);
	return d->color;
}

void Protocol::setPlayerName ( const QString& name ) {
	setAttribute ( QStringLiteral ( "PlayerName" ), name );
}

QString Protocol::playerName() const {
	return attribute ( QStringLiteral ( "PlayerName" ) ).toString();
}

void Protocol::setAttribute ( const QString& attribute, QVariant value ) {
	Q_D ( Protocol );
	d->attributes.insert ( attribute,  value );
}

void Protocol::setAttribute ( const char* attribute, QVariant value ) {
	setAttribute( QLatin1String ( attribute ), value );
}

void Protocol::setAttributes ( QVariantMap attributes ) {
	Q_D ( Protocol );
	for (auto it = attributes.constBegin(), end = attributes.constEnd(); it != end; ++it) {
		d->attributes.insert(it.key(), it.value());
	}
}

QVariant Protocol::attribute ( const QString& attribute ) const {
	Q_D ( const Protocol );
	return d->attributes.value ( attribute );
}

QVariant Protocol::attribute ( const char* attribute ) const {
	return this->attribute ( QLatin1String ( attribute ) );
}

Protocol::Features Protocol::supportedFeatures() {
	return NoFeatures;
}

int Protocol::timeRemaining() {
	return -1;
}

QList< Protocol::ToolWidgetData > Protocol::toolWidgets() {
	return QList< Protocol::ToolWidgetData >();
}

void Protocol::setWinner(Color winner) {
	Q_UNUSED(winner);
}

void Protocol::setTimeControl(const TimeControl& c) {
	Q_UNUSED(c);
}

void Protocol::acceptOffer(const Offer& offer) {
	Q_UNUSED(offer);
}

void Protocol::declineOffer(const Offer& offer) {
	Q_UNUSED(offer);
}

ChatWidget* Protocol::createChatWidget() {
	return new ChatWidget;
}

ChatWidget* Protocol::createConsoleWidget() {
	ChatWidget* console = new ChatWidget;
	console->setConsoleMode(true);
	return console;
}

void Protocol::initComplete() {
	Q_D(Protocol);
	d->ready = true;
	Q_EMIT initSuccesful();
}

bool Protocol::isReady() {
	Q_D(const Protocol);
	return d->ready;
}

bool Protocol::isLocal() {
	return false;
}

bool Protocol::isComputer() {
	return false;
}

void Protocol::setDifficulty(int depth, int memory) {
	Q_UNUSED(depth);
	Q_UNUSED(memory);
}

}

#include "moc_protocol.cpp"
