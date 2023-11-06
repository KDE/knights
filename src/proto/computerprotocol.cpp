/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "computerprotocol.h"
#include "knightsdebug.h"

#include <KProcess>
#include <KLocalizedString>

using namespace Knights;

ComputerProtocol::ComputerProtocol(QObject* parent): TextProtocol(parent), mProcess(nullptr) {

}

ComputerProtocol::~ComputerProtocol() = default;

void ComputerProtocol::startProgram() {
	QStringList args = attribute("program").toString().split ( QLatin1Char ( ' ' ) );
	QString program = args.takeFirst();
	setPlayerName ( program );
	mProcess = new KProcess ( this );
	mProcess->setProgram ( program, args );
	mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
	mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
	mProcess->setReadChannel ( KProcess::StandardOutput );
	connect ( mProcess, &KProcess::readyReadStandardError, this, &ComputerProtocol::readError );
	setDevice(mProcess);
	qCDebug(LOG_KNIGHTS) << "Starting program" << program << "with args" << args;
	mProcess->start();
	if ( !mProcess->waitForStarted ( 1000 ) ) {
		Q_EMIT error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it is installed.", program ) );
		return;
	}
}

bool ComputerProtocol::isComputer() {
	return true;
}

QList< Protocol::ToolWidgetData > ComputerProtocol::toolWidgets() {
	ChatWidget* console = createConsoleWidget();
	connect ( console, &ChatWidget::sendText, this, &ComputerProtocol::writeCheckMoves );
	setConsole ( console );
	ToolWidgetData data;
	data.widget = console;
	data.title = i18n("Console for %1 (%2)", playerName(), colorName ( color() ) );
	data.name = QLatin1String("console") + attribute("program").toString() + QLatin1Char( color() == White ? 'W' : 'B' );
	data.type = ConsoleToolWidget;
	data.owner = color();
	return {data};
}


void ComputerProtocol::readError() {
	qCCritical(LOG_KNIGHTS) << mProcess->readAllStandardError();
}
