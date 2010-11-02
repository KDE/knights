/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>

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

#include "xboardproto.h"
#include "board.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>

using namespace Knights;

XBoardProtocol::XBoardProtocol ( QObject* parent ) : Protocol ( parent )
{

}

Protocol::Features XBoardProtocol::supportedFeatures()
{
    return GameOver;
}

XBoardProtocol::~XBoardProtocol()
{
    if ( mProcess && mProcess->isOpen() )
    {
        mProcess->write ( "exit\n" );
        if ( !mProcess->waitForFinished ( 500 ) )
        {
            mProcess->kill();
        }
    }
}

void XBoardProtocol::startGame()
{

}

void XBoardProtocol::move ( const Move& m )
{
    kDebug() << m.string();
    mProcess->write ( m.string().toLatin1() + '\n' );
}

void XBoardProtocol::init ( const QVariantMap& options )
{
    setAttributes ( options );
    QStringList args = options[QLatin1String ( "program" ) ].toString().split ( QLatin1Char ( ' ' ) );
    QString program = args.takeFirst();
    if ( program.contains ( QLatin1String ( "gnuchess" ) ) && !args.contains ( QLatin1String ( "--xboard" ) ) )
    {
        args << QLatin1String ( "--xboard" );
    }
    setOpponentName ( program );
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    connect ( mProcess, SIGNAL ( readyReadStandardOutput() ), SLOT ( readFromProgram() ) );
    connect ( mProcess, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    mProcess->start();
    if ( !mProcess->waitForStarted ( 1000 ) )
    {
        emit error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it is installed.", program ) );
        return;
    }
    if ( playerColor() == NoColor )
    {
        setPlayerColor ( ( qrand() % 2 == 0 ) ? White : Black );
    }

    if ( playerColor() == Black )
    {
        mProcess->write ( "go\n" );
    }
    emit initSuccesful();
}

void XBoardProtocol::readFromProgram()
{
    QString output = QLatin1String ( mProcess->readAllStandardOutput() );
    foreach ( const QString& line, output.split ( QLatin1Char ( '\n' ) ) )
    {
        if ( line.contains ( QLatin1String ( "Illegal move" ) ) )
        {
            emit illegalMove();
        }
        else if ( line.contains ( QLatin1String ( "..." ) ) )
        {
            QString moveString = line.split ( QLatin1Char ( ' ' ) ).last();
            kDebug() << moveString;
            emit pieceMoved ( Move ( moveString ) );
        }
        else if ( line.contains ( QLatin1String ( "wins" ) ) )
        {
            Color winner;
            if ( line.split ( QLatin1Char ( ' ' ) ).last().contains ( QLatin1String ( "white" ) ) )
            {
                winner = White;
            }
            else
            {
                winner = Black;
            }
            emit gameOver ( winner );
            return;
        }
    }
}

void XBoardProtocol::readError()
{
    kError() << mProcess->readAllStandardError();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
