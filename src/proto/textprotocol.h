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


#ifndef KNIGHTS_TEXTPROTOCOL_H
#define KNIGHTS_TEXTPROTOCOL_H

#include <proto/protocol.h>
#include <QTextStream>

namespace Knights {

class TextProtocol : public Knights::Protocol
{
    Q_OBJECT
public:
    TextProtocol(QObject* parent = 0);
    virtual ~TextProtocol();

protected:
    void setDevice(QIODevice* device);

    virtual void parseLine(const QString& line) = 0;
    virtual bool parseStub(const QString& line) = 0;

protected Q_SLOTS:
    void readFromDevice();
    void write(const QString& text);
    void write(const char* text);

    void writeCheckMoves(const QString& text);

private:
    QIODevice* device;
    QTextStream stream;
};

}

#endif // KNIGHTS_TEXTPROTOCOL_H
