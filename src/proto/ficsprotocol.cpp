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


using namespace Knights;
using KWallet::Wallet;

const int Timeout = 1000; // One second ought to be enough for everybody
const QChar lineFeed = 0x0A;

FicsProtocol::FicsProtocol ( QObject* parent ) : Protocol ( parent )
{
    kDebug() << Timeout << lineFeed;
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
    kDebug() << Timeout << lineFeed;

    m_socket = new QTcpSocket ( this );
    QTextStream stream ( m_socket );
    QString address = options.value ( "address", "freechess.org" ).toString();
    int port = options.value ( "port", 5000 ).toInt();
    m_socket->connectToHost ( address, port );
    if ( !m_socket->waitForConnected ( Timeout ) )
    {
        emit error ( NetworkError, m_socket->errorString() );
        return;
    }

    KPasswordDialog passwordDialog ( qApp->activeWindow(), KPasswordDialog::ShowAnonymousLoginCheckBox | KPasswordDialog::ShowUsernameLine | KPasswordDialog::ShowKeepPassword );
    passwordDialog.setUsername ( Settings::ficsUsername() );
    if ( passwordDialog.exec() != QDialog::Accepted )
    {
        emit error ( UserCancelled );
        return;
    }
    QString username = passwordDialog.username();
    QString password = passwordDialog.password();
    bool guest = passwordDialog.anonymousMode();

    QByteArray data = m_socket->readAll();
    kDebug() << data;
    while ( !data.contains ( "login:" ) )
    {
        kDebug() << "waiting for login prompt";
        if ( !m_socket->waitForReadyRead ( Timeout ) )
        {
            emit error ( NetworkError, m_socket->errorString() );
            return;
        }
        data = m_socket->readAll();
    }
    kDebug() << "We got login prompt";
    if ( guest )
    {
        stream << "guest" << lineFeed << lineFeed;
    }
    else
    {
        stream << username << lineFeed;
        while ( !m_socket->readAll().contains ( "password:" ) )
        {
            if ( !m_socket->waitForReadyRead() )
            {
                emit error ( NetworkError, m_socket->errorString() );
                return;
            }
        }
        kDebug() << "we got password prompt";
        stream << password << lineFeed;
    }

    // We're logged in now
    kDebug() << m_socket->readAll();

    /*
     * TODO: Use KWallet for passwords
     *
    Wallet* wallet = Wallet::openWallet(Wallet::NetworkWallet(), qApp->activeWindow()->winId());
    QString folder = "Knights";
    if (!wallet->hasFolder(folder))
    {
      wallet->createFolder(folder);
    }
    wallet->setFolder(folder);
    QString password;
    wallet->readPassword(key, password);

    */

    KDialog* dialog = new KDialog;
    FicsDialog* widget = new FicsDialog;
    dialog->setMainWidget ( widget );
    dialog->setWindowModality ( Qt::WindowModal );

    kDebug() << m_socket->readAll();

    connect ( m_socket, SIGNAL ( readyRead() ), SLOT ( readFromSocket() ) );
    connect ( this, SIGNAL ( gameOfferReceived ( FicsGameOffer ) ), widget, SLOT ( addGameOffer ( FicsGameOffer ) ) );

    if ( dialog->exec() != QDialog::Accepted )
    {
        emit error ( UserCancelled );
        return;
    }
    kDebug() << m_socket->readAll();
    initSuccesful();
}

void FicsProtocol::readFromSocket()
{
    QByteArray line = m_socket->readLine();
    kDebug() << line;
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

    if ( m_socket->canReadLine() )
    {
        readFromSocket();
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;
