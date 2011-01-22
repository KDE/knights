/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "textprotocol.h"
#include <KDebug>

using namespace Knights;

TextProtocol::TextProtocol(QObject* parent): Protocol(parent)
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
	    parseLine(stream.readLine().trimmed());
	}
	else
	{
	    int n = stream.pos();
	    if ( !parseStub(stream.readAll().trimmed()) )
	    {
	        stream.seek(n);
	    }
	    return;
	}
    }
}

void TextProtocol::setDevice(QIODevice* device)
{
    stream.setDevice(device);
    connect ( device, SIGNAL(readyRead()), SLOT(readFromDevice()) );
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
    stream << text << endl;
}

void TextProtocol::write(const char* text)
{
    stream << text << endl;
}





