/***************************************************************************
    File                 : textprotocol.cpp
    Project              : Knights
    Description          : Base class for text protocols
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2017 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2009-2011 Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
#include <QIODevice>

using namespace Knights;

TextProtocol::TextProtocol(QObject* parent): Protocol(parent), m_console(nullptr) {

}

TextProtocol::~TextProtocol() = default;

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
		Q_EMIT pieceMoved(m);
	write(text);
}

void TextProtocol::write(const QString& text) {
	qCDebug(LOG_KNIGHTS) << text;
        stream << text
               << Qt::endl;
}

void TextProtocol::write(const char* text) {
	qCDebug(LOG_KNIGHTS) << text;
        stream << text
               << Qt::endl;
}

void TextProtocol::setConsole(ChatWidget* widget) {
	m_console = widget;
	for ( const ChatWidget::Message& message : std::as_const(messages) )
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

#include "moc_textprotocol.cpp"
