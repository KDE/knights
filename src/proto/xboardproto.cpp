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

#include "proto/xboardproto.h"
#include "proto/chatwidget.h"

#include <KProcess>
#include <KDebug>
#include <KLocale>
#include <KFileDialog>

using namespace Knights;

XBoardProtocol::XBoardProtocol ( QObject* parent ) : TextProtocol ( parent )
, mProcess(0)
, m_moves(0)
, m_increment(0)
, m_baseTime(0)
, m_timeLimit(0)
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
        write("quit");
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
    write ( m.string(false) );
    addMoveToHistory( m );
    lastMoveString.clear();
    emit undoPossible ( false );
    changeActivePlayer();
    if ( resumePending )
    {
        resumeGame();
    }
    startTime();
}

void XBoardProtocol::init ( const QVariantMap& options )
{
    setAttributes ( options );
    QStringList args = options[QLatin1String ( "program" ) ].toString().split ( QLatin1Char ( ' ' ) );
    QString program = args.takeFirst();
    if ( !args.contains ( QLatin1String ( "--xboard" ) ) && !args.contains ( QLatin1String ( "xboard" ) ) )
    {
        args << QLatin1String ( "xboard" );
    }
    setOpponentName ( program );
    mProcess = new KProcess ( this );
    mProcess->setProgram ( program, args );
    mProcess->setNextOpenMode ( QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text );
    mProcess->setOutputChannelMode ( KProcess::SeparateChannels );
    mProcess->setReadChannel ( KProcess::StandardOutput );
    connect ( mProcess, SIGNAL ( readyReadStandardError() ), SLOT ( readError() ) );
    setDevice(mProcess);
    kDebug() << "Starting program" << program << "with args" << args;
    mProcess->start();
    if ( !mProcess->waitForStarted ( 1000 ) )
    {
        emit error ( InstallationError, i18n ( "Program <code>%1</code> could not be started, please check that it is installed.", program ) );
        return;
    }
    TimeControl c = timeControl ( White );
    if ( c.baseTime != QTime() )
    {
        write(QString(QLatin1String("level %1 %2 %3")).arg(c.moves).arg(QTime().secsTo(c.baseTime)/60).arg(c.increment));
    }
    if ( playerColors() == NoColor )
    {
        setPlayerColor ( ( qrand() % 2 == 0 ) ? White : Black );
    }

    kDebug() << playerColors();

    if ( playerColors() & Black )
    {
        write("go");
    }
    resumePending = false;
    emit initSuccesful();
}

QList< Protocol::ToolWidgetData > XBoardProtocol::toolWidgets()
{
    m_console = createConsoleWidget();
    connect ( m_console, SIGNAL(sendText(QString)), SLOT(writeCheckMoves(QString)));
    ToolWidgetData data;
    data.widget = m_console;
    data.title = i18n("Console");
    data.name = QLatin1String("console");
    return QList< Protocol::ToolWidgetData >() << data;
}

bool XBoardProtocol::parseStub(const QString& line)
{
    parseLine(line);
    return true;
}

void XBoardProtocol::parseLine(const QString& line)
{
        kDebug() << line;
        if ( line.isEmpty() )
        {
            return;
        }
        bool display = true;
        ChatWidget::MessageType type = ChatWidget::GeneralMessage;
        if ( line.contains ( QLatin1String ( "Illegal move" ) ) )
        {
            type = ChatWidget::ErrorMessage;
            changeActivePlayer();
            emit illegalMove();
        }
        else if ( line.contains ( QLatin1String ( "..." ) ) || line.contains(QLatin1String("move")) )
        {
            kDebug() << line;
            type = ChatWidget::MoveMessage;
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
                    changeActivePlayer();
                    emit pieceMoved ( m );
                    emit undoPossible ( true );
                    startTime();
                }
            }
        }
        else if ( line.contains ( QLatin1String ( "wins" ) ) )
        {
            type = ChatWidget::StatusMessage;
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
        else if ( line.contains ( QLatin1String("offer") ) && line.contains ( QLatin1String("draw") ) )
        {
            display = false;
            if ( drawPending )
            {
                emit gameOver ( NoColor );
            }
            else
            {
                emit drawOffered();
            }
        }
        if ( display )
        {
            m_console->addText ( line, type );
        }
    }

void XBoardProtocol::readError()
{
    kError() << mProcess->readAllStandardError();
}

void XBoardProtocol::adjourn()
{
    write( QLatin1String("save") + KFileDialog::getSaveFileName() );
}

void XBoardProtocol::resign()
{
    write("resign");
}

void XBoardProtocol::undoLastMove()
{
    changeActivePlayer();
    kDebug();
    write("undo");
    emit pieceMoved(nextUndoMove());
}

void XBoardProtocol::redoLastMove()
{
    changeActivePlayer();
    Move m = nextRedoMove();
    kDebug().nospace() << m;
    write(m.string(false));
    emit pieceMoved(m);
}

void XBoardProtocol::proposeDraw()
{
    drawPending = true;
    write("draw");
}

void XBoardProtocol::pauseGame()
{
    write("force");
    stopTime();
}

void XBoardProtocol::resumeGame()
{
    if ( playerColors() & activePlayer() )
    {
        resumePending = true;
    }
    else
    {
        kDebug();
        write("go");
        emit undoPossible ( false );
        emit redoPossible ( false );
        startTime();
    }
}

void XBoardProtocol::setWinner(Color winner)
{
    QByteArray result = "result ";
    switch ( winner )
    {
        case White:
            result += "1-0";
            break;
        case Black:
            result += "0-1";
            break;
        case NoColor:
            result += "1/2-1/2";
            break;
    }
    write(QLatin1String(result));
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
