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
#include <KFileDialog>

using namespace Knights;

XBoardProtocol::XBoardProtocol ( QObject* parent ) : Protocol ( parent )
{

}

Protocol::Features XBoardProtocol::supportedFeatures()
{
    return GameOver | Draw | Adjourn | Resign | Undo | Pause;
}

XBoardProtocol::~XBoardProtocol()
{
    if ( mProcess && mProcess->isOpen() )
    {
        mProcess->write ( "quit\n" );
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
    kDebug() << "Player's move:" << m.string(false);
    m_stream << m.string(false) << endl;
    addMoveToHistory( m );
    lastMoveString.clear();
    playerActive = false;
    if ( resumePending )
    {
        resumeGame();
    }
}

void XBoardProtocol::init ( const QVariantMap& options )
{
    setAttributes ( options );
    QStringList args = options[QLatin1String ( "program" ) ].toString().split ( QLatin1Char ( ' ' ) );
    QString program = args.takeFirst();
    kDebug() << "Starting program" << program;
    if ( !args.contains ( QLatin1String ( "--xboard" ) ) && !args.contains ( QLatin1String ( "xboard" ) ) )
    {
        args << QLatin1String ( "xboard" );
    }
    setOpponentName ( program );
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    connect ( mProcess, SIGNAL ( readyReadStandardOutput() ), SLOT ( readFromProgram() ) );
    connect ( mProcess, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    mProcess->start();
    m_stream.setDevice(mProcess);
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
        m_stream << "go" << endl;
    }
    playerActive = ( playerColor() == White );
    resumePending = false;
    emit initSuccesful();
}

void XBoardProtocol::readFromProgram()
{
    QString output = m_stream.readAll();
    foreach ( const QString& line, output.split ( QLatin1Char ( '\n' ) ) )
    {
        if ( line.contains ( QLatin1String ( "Illegal move" ) ) )
        {
            emit illegalMove();
        }
        else if ( line.contains ( QLatin1String ( "..." ) ) || line.contains(QLatin1String("move")) )
        {
            const QRegExp position(QLatin1String("[a-h][1-8]"));
            if ( position.indexIn(line) > -1 )
            {
                QString moveString = line.split ( QLatin1Char ( ' ' ) ).last();
                if ( moveString != lastMoveString )
                {
                    // GnuChess may report its move twice, we need only one
                    kDebug() << "Computer's move:" << moveString;
                    lastMoveString = moveString;
                    Move m = Move ( moveString );
                    addMoveToHistory ( m );
                    emit pieceMoved ( m );
                    playerActive = true;
                }
            }
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

void XBoardProtocol::adjourn()
{
    m_stream << "save" << KFileDialog::getSaveFileName() << endl;
}

void XBoardProtocol::resign()
{
    m_stream << "resign" << endl;
}

void XBoardProtocol::undoLastMove()
{
    m_stream << "undo" << endl;
    emit pieceMoved(nextUndoMove());
}

void XBoardProtocol::redoLastMove()
{
    Move m = nextRedoMove();
    switch ( playerColor() )
    {
        // We must prevent the computer from taking over the player's side
        case White:
            m_stream << "white";
            break;
        case Black:
            m_stream << "black";
            break;
        default:
            break;
    }
    kDebug() << m.string(false);
    m_stream << m.string(false) << endl;
    m_stream << endl;
    emit pieceMoved(m);
}

void XBoardProtocol::proposeDraw()
{
}

void XBoardProtocol::pauseGame()
{
    m_stream << "force";
}

void XBoardProtocol::resumeGame()
{
    if ( playerActive )
    {
        resumePending = true;
    }
    else
    {
        m_stream << "go";
    }
}




// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
