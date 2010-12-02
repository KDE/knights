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
#include <QtCore/QPointer>


using namespace Knights;
using KWallet::Wallet;

const int FicsProtocol::Timeout = 1000; // One second ought to be enough for everybody
// TODO: Include optional [white]/[black], m, f in RegEx check

const QString FicsProtocol::namePattern = QLatin1String ( "([a-zA-z\\(\\)]+)" );
const QString FicsProtocol::ratingPattern = QLatin1String ( "([0-9\\+\\-\\s]+)" );
const QString FicsProtocol::timePattern = QLatin1String ( "(\\d+)\\s+(\\d+)" );
const QString FicsProtocol::variantPattern = QLatin1String ( "([a-z]+)\\s+([a-z]+)" );
const QString FicsProtocol::argsPattern = QLatin1String ( "(.*)" ); //TODO better
const QString FicsProtocol::idPattern = QLatin1String ( "(\\d+)" );
const QString FicsProtocol::pieces = QLatin1String ( "PRNBQKprnbqk" );
const QString FicsProtocol::coordinate = QLatin1String ( "[abdcdefgh][12345678]" );
const QString FicsProtocol::remainingTime = QLatin1String ( "\\d+ \\d+ \\d+ \\d+ \\d+ (\\d+) (\\d+) \\d+" );
const QString FicsProtocol::movePattern = QString ( QLatin1String ( "(none|o-o|o-o-o|[%2]\\/%3\\-%4(=[%5])?)" ) )
        .arg ( pieces )
        .arg ( coordinate )
        .arg ( coordinate )
        .arg ( pieces );
const QString FicsProtocol::currentPlayerPattern = QLatin1String ( "([WB]) \\-?\\d+ \\d+ \\d+ \\d+ \\d+ \\d+ \\d+" );

const QRegExp FicsProtocol::seekRegExp ( QString ( QLatin1String ( "%1 \\(%2\\) seeking %3 %4 %5\\(\"play %6\" to respond\\)" ) )
        .arg ( namePattern )
        .arg ( ratingPattern )
        .arg ( timePattern )
        .arg ( variantPattern )
        .arg ( argsPattern )
        .arg ( idPattern )
                                       );

const QRegExp FicsProtocol::soughtRegExp ( QString ( QLatin1String ( "%1 %2 %3\\s+%4\\s+%5\\s+%6" ) )
        .arg ( idPattern )
        .arg ( ratingPattern )
        .arg ( namePattern )
        .arg ( timePattern )
        .arg ( variantPattern )
        .arg ( argsPattern )
                                         );

const QRegExp FicsProtocol::moveRegExp ( QString ( QLatin1String ( "<12>.*%1.*%2 %3" ) )
        .arg ( currentPlayerPattern )
        .arg ( remainingTime )
        .arg ( movePattern )
                                       );

const QRegExp FicsProtocol::moveStringExp ( QString ( QLatin1String ( "[%1]\\/(%2)\\-(%3)(=[%4])?" ) )
        .arg ( pieces )
        .arg ( coordinate )
        .arg ( coordinate )
        .arg ( pieces )
                                          );

const QRegExp FicsProtocol::challengeRegExp ( QString ( QLatin1String ( "Challenge: %1 \\(%2\\)( \\[[a-z]+\\])? %3 \\(%4\\) %5 %6" ) )
        .arg ( namePattern )
        .arg ( ratingPattern )
        .arg ( namePattern )
        .arg ( ratingPattern )
        .arg ( variantPattern )
        .arg ( timePattern )
                                            );
const QRegExp FicsProtocol::gameStartedExp ( QString ( QLatin1String ( "Creating: %1 \\(%2\\) %3 \\(%4\\) %5 %6" ) )
        .arg ( namePattern )
        .arg ( ratingPattern )
        .arg ( namePattern )
        .arg ( ratingPattern )
        .arg ( variantPattern )
        .arg ( timePattern )
                                           );

FicsProtocol::FicsProtocol ( QObject* parent ) : Protocol ( parent )
{
    kDebug() << Timeout << endl;
    kDebug() << seekRegExp.pattern();

    forcePrompt = false;

    // FICS games are always time-limited
    setAttribute ( QLatin1String ( "TimeLimitEnabled" ), true );
}

FicsProtocol::~FicsProtocol()
{

}

Protocol::Features FicsProtocol::supportedFeatures()
{
    return TimeLimit | SetTimeLimit | UpdateTime;
}

void FicsProtocol::startGame()
{

}

void FicsProtocol::move ( const Move& m )
{
    m_stream << m.string ( false ) << endl;
}

void FicsProtocol::init ( const QVariantMap& options )
{
    setAttributes ( options );
    kDebug() << Timeout << endl;

    m_stage = ConnectStage;

    m_socket = new QTcpSocket ( this );
    m_stream.setDevice ( m_socket );
    QString address = options.value ( QLatin1String ( "address" ), QLatin1String ( "freechess.org" ) ).toString();
    int port = options.value ( QLatin1String ( "port" ), 23 ).toInt();
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

void FicsProtocol::logIn ( )
{
    username = Settings::ficsUsername();
    bool guest = ( username == QLatin1String ( "guest" ) );

    // I really hope this works on all platforms
    WId id = 0;
    if ( qApp->activeWindow() )
    {
        id = qApp->activeWindow()->winId();
    }
    Wallet* wallet = Wallet::openWallet ( Wallet::NetworkWallet(), id );

    if (wallet)
    {
        QString folder = QLatin1String ( "Knights" );
        if ( !wallet->hasFolder ( folder ) )
        {
            wallet->createFolder ( folder );
        }
        wallet->setFolder ( folder );
        QString key = username + QLatin1Char ( '@' ) + m_socket->peerName();
        wallet->readPassword ( key, password );
    }

    KPasswordDialog::KPasswordDialogFlags flags = KPasswordDialog::ShowAnonymousLoginCheckBox
                | KPasswordDialog::ShowKeepPassword
                | KPasswordDialog::ShowUsernameLine;
    QPointer<KPasswordDialog> pwDialog = new KPasswordDialog ( qApp->activeWindow(), flags );
    pwDialog->setUsername ( username );
    pwDialog->setPassword ( password );
    pwDialog->setAnonymousMode ( guest );
    if ( pwDialog->exec() == QDialog::Accepted )
    {
        guest = pwDialog->anonymousMode();
        username = pwDialog->username();
        password = pwDialog->password();
        if ( pwDialog->keepPassword() && wallet)
        {
            wallet->writePassword ( username + QLatin1Char ( '@' ) + m_socket->peerName(), password );
        }
        Settings::setFicsUsername ( username );
    }
    else
    {
        emit error ( UserCancelled );
    }
    delete pwDialog;

    if ( guest )
    {
        m_stream << "guest" << endl;
        setPlayerName ( i18n ( "You" ) );
    }
    else
    {
        m_stream << username << endl;
        setPlayerName ( username );
    }
}

void FicsProtocol::setupOptions()
{
    m_stream << "set style 12" << endl;
}

void FicsProtocol::openGameDialog()
{
    KDialog* dialog = new KDialog ( qApp->activeWindow() );
    dialog->setButtons ( KDialog::Cancel | KDialog::Apply | KDialog::Reset );
    dialog->setButtonText ( KDialog::Apply, i18n ( "Accept" ) );
    dialog->setButtonText ( KDialog::Reset, i18n ( "Decline" ) );

    m_widget = new FicsDialog ( dialog );
    dialog->setMainWidget ( m_widget );

    connect ( dialog, SIGNAL ( applyClicked() ), m_widget, SLOT ( accept() ) );
    connect ( dialog, SIGNAL ( resetClicked() ), m_widget, SLOT ( decline() ) );
    connect ( m_widget, SIGNAL ( acceptSeek ( int ) ), SLOT ( acceptSeek ( int ) ) );
    connect ( m_widget, SIGNAL ( acceptChallenge() ), SLOT ( acceptChallenge() ) );
    connect ( m_widget, SIGNAL ( declineChallenge() ), SLOT ( declineChallenge() ) );
    connect ( m_widget, SIGNAL ( declineButtonNeeded ( bool ) ), dialog->button ( KDialog::Reset ), SLOT ( setEnabled ( bool ) ) );

    connect ( this, SIGNAL ( gameOfferReceived ( FicsGameOffer ) ), m_widget, SLOT ( addGameOffer ( FicsGameOffer ) ) );
    connect ( this, SIGNAL ( challengeReceived ( FicsPlayer ) ), m_widget, SLOT ( addChallenge ( FicsPlayer ) ) );
    connect ( m_widget, SIGNAL ( sought() ), SLOT ( checkSought() ) );
    connect ( m_widget, SIGNAL ( seekingChanged ( bool ) ), SLOT ( setSeeking ( bool ) ) );

    // connect ( dialog, SIGNAL(accepted()), SLOT(dialogAccepted()));
    connect ( dialog, SIGNAL ( rejected() ), SLOT ( dialogRejected() ) );

    connect ( this, SIGNAL ( initSuccesful() ), dialog, SLOT ( accept() ) );
    connect ( this, SIGNAL ( error ( Protocol::ErrorCode, QString ) ), dialog, SLOT ( deleteLater() ) );
    dialog->show();
}

void FicsProtocol::readFromSocket()
{
    if ( !m_socket->canReadLine() )
    {
        QByteArray next = m_socket->peek ( 10 );
        if ( !next.contains ( "fics" ) && !next.contains ( "login:" ) && !next.contains ( "password:" ) )
        {
            // It is neither a prompt nor a complete line, so we wait for more data
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
                kDebug() << line;
                m_stage = SeekStage;
                setupOptions();
                openGameDialog();
                QString name = QLatin1String ( line );
                name.remove ( 0, name.indexOf ( QLatin1String ( "session as " ) ) + 11 );
                if ( name.contains ( QLatin1String ( "(U)" ) ) )
                {
                    name.truncate ( name.indexOf ( QLatin1String ( "(U)" ) ) );
                }
                else
                {
                    name.truncate ( name.indexOf ( QLatin1Char ( ' ' ) ) );
                }
                kDebug() << name;
                setPlayerName ( name );
            }
            else if ( line.contains ( "Invalid password" ) )
            {
                forcePrompt = true;
            }
            break;
        case SeekStage:
            if ( seekRegExp.indexIn ( QLatin1String ( line ) ) != -1 )
            {
                FicsGameOffer offer;
                int n = 1;
                offer.player.first = seekRegExp.cap ( n++ );
                offer.player.second = seekRegExp.cap ( n++ ).toInt();
                offer.baseTime = seekRegExp.cap ( n++ ).toInt();
                offer.timeIncrement = seekRegExp.cap ( n++ ).toInt();
                offer.rated = ( seekRegExp.cap ( n++ ) == QLatin1String ( "rated" ) );
                offer.variant = seekRegExp.cap ( n++ );
                QString extraParams = seekRegExp.cap ( n++ );
                offer.gameId = seekRegExp.cap ( n++ ).toInt();
                emit gameOfferReceived ( offer );
            }
            else if ( soughtRegExp.indexIn ( QLatin1String ( line ) ) != -1 )
            {
                kDebug() << "sought:" << line << soughtRegExp.cap();
                FicsGameOffer offer;
                for (int i = 0; i < soughtRegExp.numCaptures(); ++i)
                {
                    kDebug() << soughtRegExp.cap(i);
                }
                int n = 1;
                offer.gameId = soughtRegExp.cap ( n++ ).toInt();
                offer.player.second = soughtRegExp.cap ( n++ ).toInt();
                offer.player.first = soughtRegExp.cap ( n++ );
                offer.baseTime = soughtRegExp.cap ( n++ ).toInt();
                offer.timeIncrement = soughtRegExp.cap ( n++ ).toInt();
                offer.rated = ( soughtRegExp.cap ( n++ ) == QLatin1String ( "rated" ) );
                offer.variant = soughtRegExp.cap ( n++ );
                // TODO: The rest
                emit gameOfferReceived ( offer );
            }
            else if ( challengeRegExp.indexIn ( QLatin1String ( line ) ) > -1 )
            {
                FicsPlayer player;
                player.first = challengeRegExp.cap ( 1 );
                player.second = challengeRegExp.cap ( 2 ).toInt();
                emit challengeReceived ( player );
            }
            else if ( gameStartedExp.indexIn ( QLatin1String ( line ) ) > -1 )
            {
                QString player1 = gameStartedExp.cap ( 1 );
                QString player2 = gameStartedExp.cap ( 3 );
                if ( player1 == playerName() )
                {
                    setPlayerColor ( White );
                    setOpponentName ( player2 );
                }
                else
                {
                    setPlayerColor ( Black );
                    setOpponentName ( player1 );
                }
                m_stage = PlayStage;
                emit initSuccesful();
            }
            break;
        case PlayStage:
            if ( moveRegExp.indexIn ( QLatin1String ( line ) ) > -1 )
            {
                if ( ( moveRegExp.cap ( 1 ) == QLatin1String ( "B" ) && playerColor() == White )
                        || ( moveRegExp.cap ( 1 ) == QLatin1String ( "W" ) && playerColor() == Black ) )
                {
                    // It is not our turn now
                    // This is only the confirmation of our previous move, ignore it
                    break;
                }
                const int whiteTimeLimit = moveRegExp.cap ( 2 ).toInt();
                const int blackTimeLimit = moveRegExp.cap ( 3 ).toInt();
                const QString moveString = moveRegExp.cap ( 4 );

                kDebug() << moveString;

                if ( moveString == QLatin1String ( "none" ) )
                {
                    if ( playerColor() == White )
                    {
                        setAttribute ( QLatin1String ( "playerTimeLimit" ), QTime().addSecs ( whiteTimeLimit ) );
                        setAttribute ( QLatin1String ( "oppTimeLimit" ), QTime().addSecs ( blackTimeLimit ) );
                    }
                    else
                    {
                        setAttribute ( QLatin1String ( "playerTimeLimit" ), QTime().addSecs ( blackTimeLimit ) );
                        setAttribute ( QLatin1String ( "oppTimeLimit" ), QTime().addSecs ( whiteTimeLimit ) );
                    }
                    break;
                }

                Move m;
                if ( moveString == QLatin1String ( "o-o" ) )
                {
                    // Short (king's rook) castling
                    m = Move::castling ( Move::KingSide, oppositeColor ( playerColor() ) );
                }
                else if ( moveString == QLatin1String ( "o-o-o" ) )
                {
                    // Long (Queen's rock) castling
                    m = Move::castling ( Move::QueenSide, oppositeColor ( playerColor() ) );
                }
                else if ( moveStringExp.indexIn ( moveString ) > -1 )
                {
                    m.setFrom ( moveStringExp.cap ( 1 ) );
                    m.setTo ( moveStringExp.cap ( 2 ) );
                    if ( moveStringExp.numCaptures() > 2 )
                    {
                        m.setFlag ( Move::Promote, true );
                        QChar typeChar = moveRegExp.cap ( 3 ).mid ( 1, 1 ) [0];
                        m.setPromotedType ( Piece::typeFromChar ( typeChar ) );
                    }
                }
                emit timeChanged ( White, QTime().addSecs ( whiteTimeLimit ) );
                emit timeChanged ( Black, QTime().addSecs ( blackTimeLimit ) );
                emit pieceMoved ( m );
            }
            else if ( line.contains ( "lost contact or quit" ) )
            {
                emit gameOver ( NoColor );
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

void FicsProtocol::acceptSeek ( int id )
{
    m_stream << "play " << id << endl;
    m_seeking = false;
}

void FicsProtocol::acceptChallenge()
{
    m_stream << "accept" << endl;
    m_seeking = true;
}

void FicsProtocol::declineChallenge()
{
    m_stream << "decline" << endl;
}

void FicsProtocol::dialogAccepted()
{

}

void FicsProtocol::dialogRejected()
{
    emit error ( UserCancelled );
}

void FicsProtocol::setSeeking ( bool seek )
{
    m_seeking = seek;
    if ( seek )
    {
        m_stream << "seek";
        if ( attribute ( QLatin1String ( "playerTimeLimit" ) ).canConvert<QTime>() && attribute ( QLatin1String ( "playerTimeIncrement" ) ).canConvert<int>() )
        {
            QTime time = attribute ( QLatin1String ( "playerTimeLimit" ) ).toTime();
            m_stream << ' ' << 60 * time.hour() + time.minute();
            m_stream << ' ' << attribute ( QLatin1String ( "playerTimeIncrement" ) ).toInt();
        }
        m_stream << " unrated"; // TODO: Option for this
        switch ( playerColor() )
        {
            case White:
                m_stream << " white";
                break;
            case Black:
                m_stream << " black";
                break;
            default:
                break;
        }
        m_stream << " manual";
    }
    else
    {
        m_stream << "unseek";
    }
    m_stream << endl;
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
