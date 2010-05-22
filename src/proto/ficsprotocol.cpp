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

#include "ficsprotocol.h"

#include "proto/ficsdialog.h"
#include "settings.h"

#include <KDialog>
#include <KWallet/Wallet>
#include <KPasswordDialog>
#include <KLocale>

#include <QtNetwork/QTcpSocket>
#include <QtGui/QApplication>
#include <gamedialog.h>


using namespace Knights;
using KWallet::Wallet;

const int Timeout = 1000; // One second ought to be enough for everybody
// const QString endl = QString(QChar(0x0A)) + QChar('\n');
const QRegExp seekRegExp = QRegExp ( "([a-zA-z])+ \\(([0-9\\+]{4,4})\\) seeking ([a-z]+) ([a-z]+) (f?) (m?) \\(\\\"play ([0-9]+)\\\" to respond\\)" );

FicsProtocol::FicsProtocol ( QObject* parent ) : Protocol ( parent )
{
    kDebug() << Timeout << endl;
}

FicsProtocol::~FicsProtocol()
{

}

Protocol::Features FicsProtocol::supportedFeatures()
{
    return TimeLimit | ChangeTimeLimit;
}

void FicsProtocol::startGame()
{

}

void FicsProtocol::move ( const Move& m )
{

}

void FicsProtocol::init ( const QVariantMap& options )
{
    kDebug() << Timeout << endl;

    m_stage = ConnectStage;

    m_socket = new QTcpSocket ( this );
    m_stream.setDevice ( m_socket );
    QString address = options.value ( "address", "freechess.org" ).toString();
    int port = options.value ( "port", 23 ).toInt();
    connect ( m_socket, SIGNAL ( connected() ), SLOT ( socketConnected() ) );
    connect ( m_socket, SIGNAL ( error ( QAbstractSocket::SocketError ) ), SLOT ( socketError() ) );
    connect ( m_socket, SIGNAL ( readyRead() ), SLOT ( readFromSocket() ) );
    m_socket->connectToHost ( address, port );
}

void FicsProtocol::socketConnected()
{
    kDebug();
}

void FicsProtocol::socketError()
{
    kDebug() << m_socket->errorString();
}

void FicsProtocol::logIn ( bool forcePrompt )
{
    username = Settings::ficsUsername();
    Wallet* wallet = Wallet::openWallet ( Wallet::NetworkWallet(), qApp->activeWindow()->winId() );
    QString folder = "Knights";
    if ( !wallet->hasFolder ( folder ) )
    {
        wallet->createFolder ( folder );
    }
    wallet->setFolder ( folder );
    QString key = username + '@' + m_socket->peerName();
    wallet->readPassword ( username, password );
    bool guest = username == "guest";
    if ( forcePrompt || username.isEmpty() || password.isEmpty() )
    {
        KPasswordDialog::KPasswordDialogFlags flags = KPasswordDialog::ShowAnonymousLoginCheckBox | KPasswordDialog::ShowKeepPassword | KPasswordDialog::ShowUsernameLine;
        KPasswordDialog pwDialog ( qApp->activeWindow(), flags );
        pwDialog.setUsername ( username );
        if ( pwDialog.exec() == QDialog::Accepted )
        {
            guest = pwDialog.anonymousMode();
            username = pwDialog.username();
            password = pwDialog.password();
            if ( pwDialog.keepPassword() )
            {
                wallet->writePassword ( username + '@' + m_socket->peerName(), password );
            }
        }
    }
    kDebug() << username;
    if ( guest )
    {
        m_stream << "guest" << endl;
    }
    else
    {
        m_stream << username << endl;
    }
}

void FicsProtocol::openGameDialog()
{
    KDialog* dialog = new KDialog ( qApp->activeWindow() );
    FicsDialog* widget = new FicsDialog ( dialog );
    dialog->setMainWidget ( widget );

    connect ( this, SIGNAL ( gameOfferReceived ( FicsGameOffer ) ), widget, SLOT ( addGameOffer ( FicsGameOffer ) ) );
    dialog->show();
}

void FicsProtocol::readFromSocket()
{
    QByteArray line = m_socket->readLine();
    kDebug() << line << seekRegExp.indexIn ( line );
    switch ( m_stage )
    {
        case ConnectStage:
            if ( line.contains ( "login:" ) )
            {
                logIn();
            }
            else if ( line.contains ( "password:" ) )
            {
                m_stream << password << endl;
            }
            else if ( line.contains ( "Guest" ) )
            {
                m_stream << endl;
            }
            // TODO: Check for incorrect logins
            else if ( line.contains ( "Starting FICS session" ) )
            {
                m_stage = SeekStage;
                openGameDialog();
            }
            break;
        case SeekStage:
            if ( line.contains ( "seeking" ) )
            {
                FicsGameOffer offer;
                QRegExp idRegExp ( "play ([1-9][0-9]*)" );
                idRegExp.indexIn ( line );
                offer.gameId = idRegExp.cap().toInt();

                QList<QByteArray> seekArgs = line.split ( ' ' );

                offer.player = seekArgs.takeFirst();
                offer.rating = seekArgs.takeFirst().toInt();
                if ( seekArgs.takeFirst() != QByteArray ( "seeking" ) )
                {
                    return;
                }
                offer.baseTime = seekArgs.takeFirst().toInt();
                offer.timeIncrement = seekArgs.takeFirst().toInt();
                offer.rated = ( seekArgs.takeFirst() == QByteArray ( "rated" ) );
                offer.variant = seekArgs.takeFirst();
                QByteArray next = seekArgs.takeFirst();
                if ( next.startsWith ( '[' ) && next.endsWith ( ']' ) )
                {
                    if ( next.contains ( "[white]" ) )
                    {
                        offer.color = Piece::Black;
                    }
                    else if ( next.contains ( "[black]" ) )
                    {
                        offer.color = Piece::White;
                    }
                    next = seekArgs.takeFirst();
                }
                else
                {
                    offer.color = Piece::NoColor;
                }
                if ( next == "m" )
                {
                    offer.manual = true;
                    next = seekArgs.takeFirst();
                }
                if ( next == "f" )
                {
                    offer.formula = true;
                    next = seekArgs.takeFirst();
                }
                emit gameOfferReceived ( offer );
            }
            break;
    }

    if ( m_socket->bytesAvailable() > 0 )
    {
        readFromSocket();
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
