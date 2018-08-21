/***************************************************************************
    File                 : textprotocol.h
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

#ifndef KNIGHTS_TEXTPROTOCOL_H
#define KNIGHTS_TEXTPROTOCOL_H

#include <proto/protocol.h>
#include "chatwidget.h"

#include <QList>
#include <QTextStream>

namespace Knights {

class TextProtocol : public Protocol {
	Q_OBJECT
public:
	explicit TextProtocol(QObject* parent = nullptr);
	virtual ~TextProtocol();

protected:
	void setDevice(QIODevice*);
	QIODevice* device() const;

	void setConsole(ChatWidget*);
	ChatWidget* console() const;

	void writeToConsole(const QString&, ChatWidget::MessageType);

	virtual bool parseLine(const QString&) = 0;
	virtual bool parseStub(const QString&) = 0;

protected Q_SLOTS:
	void readFromDevice();
	void write(const QString&);
	void write(const char*);
	void writeCheckMoves(const QString&);

private:
	QTextStream stream;
	QPointer<ChatWidget> m_console;
	QList<ChatWidget::Message> messages;
	QString line;
};

}

#endif // KNIGHTS_TEXTPROTOCOL_H
