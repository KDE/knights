/***************************************************************************
    File                 : textprotocol.cpp
    Project              : Knights
    Description          : Base class for text protocols
    --------------------------------------------------------------------
    Copyright            : (C) 2017 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2009-2011 by Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "textprotocol.h"
#include "knightsdebug.h"

using namespace Knights;

TextProtocol::TextProtocol(QObject* parent): Protocol(parent), m_console(0) {

}

TextProtocol::~TextProtocol() {

}

void TextProtocol::readFromDevice() {
	while ( !stream.atEnd() ) {
		if ( stream.device()->canReadLine() ) {
			const QString text = line + stream.readLine().trimmed();
			if (!parseLine(text))
				line = text;
		} else {
			const QString all = stream.readAll();
			for ( const QString& newLine : all.split( QLatin1Char('\n') ) ) {
				const QString text = line + newLine.trimmed();
				if ( !parseStub(text) && !parseLine(text) )
					line = text;
			}
		}
	}
}

void TextProtocol::setDevice(QIODevice* device) {
	stream.setDevice(device);
	connect ( device, &QIODevice::readyRead, this, &TextProtocol::readFromDevice );
}

QIODevice* TextProtocol::device() const {
	return stream.device();
}

void TextProtocol::writeCheckMoves(const QString& text) {
	Move m = Move(text);
	if ( m.isValid() )
		emit pieceMoved(m);
	write(text);
}

void TextProtocol::write(const QString& text) {
	qCDebug(LOG_KNIGHTS) << text;
	stream << text << endl;
}

void TextProtocol::write(const char* text) {
	qCDebug(LOG_KNIGHTS) << text;
	stream << text << endl;
}

void TextProtocol::setConsole(ChatWidget* widget) {
	m_console = widget;
	for ( const ChatWidget::Message& message : messages )
		m_console->addText ( message );
	messages.clear();
}

ChatWidget* TextProtocol::console() const {
	return m_console;
}

void TextProtocol::writeToConsole(const QString& text, ChatWidget::MessageType type) {
	if ( m_console )
		m_console->addText ( text, type );
	else
		messages << qMakePair( text, type );
}
