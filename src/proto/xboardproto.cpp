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
        mProcess->write("exit\n");
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
    QString move;
    move.append ( m.from().string() );
    if ( m.flags() & Move::Take )
    {
        move.append ( 'x' );
    }
    move.append ( m.to().string() );
    move.append ( '\n' );
    kDebug() << move;
    mProcess->write ( move.toLatin1() );
}

void XBoardProtocol::init ( const QVariantMap& options )
{
    setAttributes(options);
    QStringList args = options["program"].toString().split ( ' ' );
    QString program = args.takeFirst();
    if ( program.contains ( "gnuchess" ) && !args.contains ( "--xboard" ) )
    {
        args << "--xboard";
    }
    setOpponentName(program);
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    connect ( mProcess, SIGNAL ( readyReadStandardOutput() ), SLOT ( readFromProgram() ) );
    connect ( mProcess, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    mProcess->start();
    if ( !mProcess->waitForStarted ( 1000 ) )
    {
        emit error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it's installed", program ) );
        return;
    }
    if ( playerColor() == NoColor )
    {
        setPlayerColor( ( qrand() % 2 == 0 ) ? White : Black );
    }

    if ( playerColor() == Black )
    {
        mProcess->write("go\n");
    }
    emit initSuccesful();
}

void XBoardProtocol::readFromProgram()
{
    QString output = QString ( mProcess->readAllStandardOutput() );
    foreach (const QString& line, output.split('\n'))
    {
        if ( line.contains ( "Illegal move" ) )
        {
            emit illegalMove();
        }
        else if (line.contains( "..." ))
        {
            QString moveString = line.split ( ' ' ).last();
            kDebug() << moveString;
            Move m;
            m.setFrom ( Pos ( moveString.left ( 2 ) ) );
            m.setFlag( Move::Take, moveString.contains ( 'x' ) );
            m.setTo ( Pos ( moveString.right ( 2 ) ) );
            emit pieceMoved ( m );
        }
        else if ( line.contains ( "wins" ) )
        {
            Color winner;
            if ( line.split ( ' ' ).last().contains ( "white" ) )
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

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
