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

XBoardProtocol::XBoardProtocol ( QObject* parent ) : Protocol ( parent )
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
    emit undoPossible ( false );
    playerActive = !playerActive;
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

    TimeControl c = timeControl ( White );
    m_stream << "level " << c.moves << ' ' << QTime().secsTo(c.baseTime)/60 << ' ' << c.increment;

    if ( playerColors() == NoColor )
    {
        setPlayerColor ( ( qrand() % 2 == 0 ) ? White : Black );
    }

    if ( playerColors() & Black )
    {
        m_stream << "go" << endl;
    }
    playerActive = ( playerColors() & White );
    resumePending = false;
    emit initSuccesful();
}

QList< Protocol::ToolWidgetData > XBoardProtocol::toolWidgets()
{
    m_console = createConsoleWidget();
    connect ( m_console, SIGNAL(sendText(QString)), SLOT(writeToProgram(QString)));
    ToolWidgetData data;
    data.widget = m_console;
    data.title = i18n("Console");
    data.name = QLatin1String("console");
    return QList< Protocol::ToolWidgetData >() << data;
}


void XBoardProtocol::readFromProgram()
{
    QString output = m_stream.readAll();
    foreach ( const QString& line, output.split ( QLatin1Char ( '\n' ) ) )
    {
        if ( line.isEmpty() )
        {
            continue;
        }
        bool display = true;
        ChatWidget::MessageType type = ChatWidget::GeneralMessage;
        if ( line.contains ( QLatin1String ( "Illegal move" ) ) )
        {
            type = ChatWidget::ErrorMessage;
            playerActive = true;
            emit illegalMove();
        }
        else if ( line.contains ( QLatin1String ( "..." ) ) || line.contains(QLatin1String("move")) )
        {
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
                    playerActive = !playerActive;
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
}

void XBoardProtocol::writeToProgram ( const QString& text )
{
    if ( playerActive )
    {
        Move m = Move(text);
        if ( m.isValid() )
        {
            emit pieceMoved ( m );
            return;
        }
    }
    m_stream << text << endl;
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
    playerActive = !playerActive;
    kDebug();
    m_stream << "undo" << endl;
    emit pieceMoved(nextUndoMove());
}

void XBoardProtocol::redoLastMove()
{
    playerActive = !playerActive;
    Move m = nextRedoMove();
    kDebug().nospace() << m;
    m_stream << m.string(false) << endl;
    emit pieceMoved(m);
}

void XBoardProtocol::proposeDraw()
{
    drawPending = true;
    m_stream << "draw" << endl;
}

void XBoardProtocol::pauseGame()
{
    kDebug();
    m_stream << "force" << endl;
    stopTime();
}

void XBoardProtocol::resumeGame()
{
    if ( playerActive )
    {
        resumePending = true;
    }
    else
    {
        kDebug();
        m_stream << "go" << endl;
        emit undoPossible ( false );
        emit redoPossible ( false );
        startTime();
    }
}

void XBoardProtocol::setTimeControl(Color color, int moves, const QTime& baseTime, int increment)
{
    if ( mProcess && mProcess->isOpen() )
    {
        kDebug() << moves << baseTime << increment;
        m_stream << QString(QLatin1String("level %1 %2 %3")).arg(moves).arg(m_baseTime).arg(increment) << endl;
    }
    Protocol::setTimeControl ( color, moves, baseTime, increment );
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
    m_stream << result << endl;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
