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

using namespace Knights;

XBoardProtocol::XBoardProtocol(QObject* parent): Protocol(parent)
{

}

Protocol::Features XBoardProtocol::supportedFeatures()
{
    return NoFeatures;
}

XBoardProtocol::~XBoardProtocol()
{
  if (mProcess && mProcess->isOpen())
  {
    QTextStream stream(mProcess);
    stream << "exit\n";
    if (!mProcess->waitForFinished(500))
    {
      mProcess->kill();
    }
  }
}


void XBoardProtocol::startGame()
{

}

void XBoardProtocol::move ( Move m )
{
    QString move;
    move.append(Board::row(m.from().first));
    move.append(QString::number(m.from().second));
    if (m.flags() & Move::Take)
    {
        move.append('x');
    }
    move.append(Board::row(m.to().first));
    move.append(QString::number(m.to().second));
    move.append('\n');
    kDebug() << move;
    mProcess->write(move.toLatin1());
}

void XBoardProtocol::setPlayerColor(Piece::Color color)
{
    mPlayerColor = color;
}


bool XBoardProtocol::init(QVariantMap options)
{
    QStringList args = options["program"].toString().split(' ');
    QString program = args.takeFirst();
    if (program.contains("gnuchess") && !args.contains("--xboard"))
    {
        args << "--xboard";
    }
    mProcess = new KProcess;
    mProcess->setProgram(program, args);
    mProcess->setNextOpenMode(QIODevice::ReadWrite | QIODevice::Unbuffered | QIODevice::Text);
    mProcess->setOutputChannelMode(KProcess::SeparateChannels);
    connect(mProcess, SIGNAL(readyReadStandardOutput()), SLOT(readFromProgram()));
    connect(mProcess, SIGNAL(readyReadStandardError()), SLOT(readError()));
    mProcess->start();
    QTextStream stream(mProcess);
    if (!mProcess->waitForStarted(1000))
    {
        qCritical() << "Program" << program << "could not be started";
        return false;
    }
    if (mPlayerColor == Piece::Black)
    {
        stream << "go\n";
    }
    return true;
}

void XBoardProtocol::readFromProgram()
{
    QString moveString = QString(mProcess->readAllStandardOutput());
    kDebug() << moveString;
    if (!moveString.contains("..."))
    {
      if (moveString.contains("Illegal move"))
      {
        emit illegalMove();
      }
      return;
    }
    if (moveString.contains("wins"))
    {
      if (moveString.split(' ').last().contains("white"))
      {
        emit gameOver(Piece::White);
      }
      else
      {
        emit gameOver(Piece::Black);
      }
      return;
    }
    moveString = moveString.split(' ').last();
    Move m;
    m.setFrom(Pos(Board::numFromRow(moveString[0]),moveString.mid(1,1).toInt()));
    int i = 2;
    if (moveString.contains('x'))
    {
        m.setFlag(Move::Take, true);
        i++;
    }
    else
    {
        m.setFlag(Move::Take, false);
    }
    m.setTo(Pos(Board::numFromRow(moveString[i]),moveString.mid(i+1,1).toInt()));

    kDebug() << moveString << m.from() << m.to();
    emit pieceMoved(m);
}

void XBoardProtocol::readError()
{
    kError() << mProcess->readAllStandardError();
}

