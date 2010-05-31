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
#include <KPushButton>

#include <QtNetwork/QTcpSocket>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <gamedialog.h>


using namespace Knights;
using KWallet::Wallet;

const int FicsProtocol::Timeout = 1000; // One second ought to be enough for everybody
// TODO: Include optional [white]/[black], m, f in RegEx check
const QRegExp FicsProtocol::seekRegExp("([a-zA-z]+) \\(([0-9\\+]+)\\) seeking ([\\d]+) ([\\d]+) ([a-z]+) ([a-z]+)(.*)\\(\"play ([\\d]+)\" to respond\\)");
const QRegExp FicsProtocol::soughtRegExp("([0-9]+) ([0-9\\+]+) ([a-zA-z\\(\\)]+)\\s+([0-9]+)\\s+([0-9]+) ([a-z]+) ([a-z]+) (.*)([0-9]+)-([0-9]+)(.*)");
const QRegExp FicsProtocol::moveRegExp("<12> (.*)");
const QRegExp FicsProtocol::challengeRegExp("challenge");

FicsProtocol::FicsProtocol ( QObject* parent ) : Protocol ( parent )
{
    kDebug() << Timeout << endl;
    kDebug() << seekRegExp.pattern();
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
        KPasswordDialog::KPasswordDialogFlags flags = KPasswordDialog::ShowAnonymousLoginCheckBox 
                                                    | KPasswordDialog::ShowKeepPassword 
                                                    | KPasswordDialog::ShowUsernameLine;
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
    dialog->setButtons(KDialog::Cancel | KDialog::Apply | KDialog::Reset);
    dialog->setButtonText(KDialog::Apply, i18n("Accept"));
    dialog->setButtonText(KDialog::Reset, i18n("Decline"));
    
    m_widget = new FicsDialog ( dialog );
    dialog->setMainWidget ( m_widget );

    connect ( dialog, SIGNAL(applyClicked()), m_widget, SLOT(accept()));
    connect ( dialog, SIGNAL(resetClicked()), m_widget, SLOT(decline()));
    connect ( m_widget, SIGNAL(declineButtonNeeded(bool)), dialog->button(KDialog::Reset), SLOT(setEnabled(bool)));
    
    connect ( this, SIGNAL ( gameOfferReceived ( FicsGameOffer ) ), m_widget, SLOT ( addGameOffer ( FicsGameOffer ) ) );
    connect ( m_widget, SIGNAL ( sought() ), SLOT ( checkSought() ) );
    connect ( m_widget, SIGNAL ( seek() ), SLOT ( seek() ) );
    
    connect ( dialog, SIGNAL(accepted()), SLOT(dialogAccepted())); 
    connect ( dialog, SIGNAL(rejected()), SLOT(dialogRejected()));
    dialog->show();
}

void FicsProtocol::readFromSocket()
{
    if (!m_socket->canReadLine())
    {
        QByteArray next = m_socket->peek(10);
        if (!next.contains("fics") && !next.contains("login:") && !next.contains("password:"))
        {
            // It is neither a prompt nor a complete line, so there will be more data to read soon
            return;
        }
    }
    QByteArray line = m_socket->readLine();
    kDebug() << line;
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
            else if ( line.contains ( "Press return to enter the server" ) )
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
            if (seekRegExp.indexIn(line) != -1)
            {
                FicsGameOffer offer;
                int n = 1;
                offer.player = seekRegExp.cap(n++);
                offer.rating = seekRegExp.cap(n++).toInt();
                offer.baseTime = seekRegExp.cap(n++).toInt();
                offer.timeIncrement = seekRegExp.cap(n++).toInt();
                offer.rated = (seekRegExp.cap(n++) == "rated");
                offer.variant = seekRegExp.cap(n++);
                QString extraParams = seekRegExp.cap(n++);
                offer.gameId = seekRegExp.cap(n++).toInt();
                emit gameOfferReceived ( offer );
            }
            else if (soughtRegExp.indexIn(line) != -1)
            {
                FicsGameOffer offer;
                int n = 1;
                offer.gameId = soughtRegExp.cap(n++).toInt();
                offer.rating = soughtRegExp.cap(n++).toInt();
                offer.player = soughtRegExp.cap(n++);
                offer.baseTime = soughtRegExp.cap(n++).toInt();
                offer.timeIncrement = soughtRegExp.cap(n++).toInt();
                offer.rated = (soughtRegExp.cap(n++) == "rated");
                offer.variant = soughtRegExp.cap(n++);
                // TODO: The rest
                emit gameOfferReceived ( offer );
            }
            else if (challengeRegExp.indexIn(line) > -1)
            {
                // emit challengeReceived();
            }
            break;
        case PlayStage:
            if (moveRegExp.indexIn(line) > -1)
            {
                QStringList args = moveRegExp.cap().split(' ');
                if (args.size() < 30)
                {
                    return;
                }
            }
    }

    if ( m_socket->bytesAvailable() > 0 )
    {
        readFromSocket();
    }
}

void FicsProtocol::checkSought()
{
    m_stream << "sought" << endl;
}

void FicsProtocol::dialogAccepted()
{
    
}

void FicsProtocol::dialogRejected()
{
    emit error(UserCancelled);
}

void FicsProtocol::setSeeking(bool seek)
{
    if (seek)
    {
        m_stream << "seek" << endl;
    }
    else
    {
        m_stream << "unseek" << endl;
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;
