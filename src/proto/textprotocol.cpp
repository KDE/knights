/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>

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

#include "textprotocol.h"
#include <KDebug>

using namespace Knights;

TextProtocol::TextProtocol(QObject* parent): Protocol(parent), m_console(0)
{

}

TextProtocol::~TextProtocol()
{

}

void TextProtocol::readFromDevice()
{
    while ( !stream.atEnd() )
    {
	if ( stream.device()->canReadLine() )
	{
            const QString text = line + stream.readLine().trimmed();
	    if (!parseLine(text))
            {
                line = text;
            }
	}
	else
	{
	    foreach ( const QString& newLine, stream.readAll().split( QLatin1Char('\n') ) )
	    {
                const QString text = line + newLine.trimmed();
                if ( !parseStub(text) && !parseLine(text) )
                {
                    line = text;
                }
            }
	}
    }
}

void TextProtocol::setDevice(QIODevice* device)
{
    stream.setDevice(device);
    connect ( device, SIGNAL(readyRead()), SLOT(readFromDevice()) );
}

QIODevice* TextProtocol::device() const
{
    return stream.device();
}

void TextProtocol::writeCheckMoves(const QString& text)
{
    Move m = Move(text);
    if ( m.isValid() )
    {
      emit pieceMoved(m);
    }
    write(text);
}

void TextProtocol::write(const QString& text)
{
    kDebug() << text;
    stream << text << endl;
}

void TextProtocol::write(const char* text)
{
    kDebug() << text;
    stream << text << endl;
}

void TextProtocol::setConsole(ChatWidget* widget)
{
    m_console = widget;
    foreach ( const ChatWidget::Message& message, messages )
    {
      m_console->addText ( message );
    }
    messages.clear();
}

ChatWidget* TextProtocol::console() const
{
    return m_console;
}

void TextProtocol::writeToConsole(const QString& text, ChatWidget::MessageType type)
{
    if ( m_console )
    {
      m_console->addText ( text, type );
    }
    else
    {
      messages << qMakePair( text, type );
    }
}






