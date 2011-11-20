/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2011  Miha Čančula <miha@noughmad.eu>

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

#include "computerprotocol.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>

using namespace Knights;

ComputerProtocol::ComputerProtocol(QObject* parent): TextProtocol(parent)
{

}

ComputerProtocol::~ComputerProtocol()
{

}

void ComputerProtocol::startProgram()
{
    QStringList args = attribute("program").toString().split ( QLatin1Char ( ' ' ) );
    QString program = args.takeFirst();
    setPlayerName ( program );
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    mProcess->setReadChannel ( KProcess::StandardOutput );
    connect ( mProcess, SIGNAL (readyReadStandardError()), SLOT (readError()) );
    setDevice(mProcess);
    kDebug() << "Starting program" << program << "with args" << args;
    mProcess->start();
    if ( !mProcess->waitForStarted ( 1000 ) )
    {
        emit error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it is installed.", program ) );
        return;
    }
}

bool ComputerProtocol::isComputer()
{
    return true;
}

QList< Protocol::ToolWidgetData > ComputerProtocol::toolWidgets()
{
    ChatWidget* console = createConsoleWidget();
    connect ( console, SIGNAL(sendText(QString)), SLOT(writeCheckMoves(QString)));
    setConsole ( console );
    ToolWidgetData data;
    data.widget = console;
    data.title = i18n("Console for %1 (%2)", playerName(), colorName ( color() ) );
    data.name = QLatin1String("console") + attribute("program").toString() + QLatin1Char( color() == White ? 'W' : 'B' );
    data.type = ConsoleToolWidget;
    data.owner = color();
    return QList< Protocol::ToolWidgetData >() << data;
}



