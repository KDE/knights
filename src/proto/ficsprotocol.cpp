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

#include <KPluginFactory>
#include <KPluginLoader>
#include <KParts/ReadOnlyPart>
#include <kde_terminal_interface_v2.h>

#include <QtNetwork/QTcpSocket>
#include <QtGui/QApplication>
#include <QtGui/QPushButton>
#include <QtCore/QPointer>
#include <QtGui/QDockWidget>
#include <QtGui/QMainWindow>
#include <knights.h>
#include <KActionCollection>
#include "keyboardeventfilter.h"

using namespace Knights;
using KWallet::Wallet;

const int FicsProtocol::Timeout = 1000; // One second ought to be enough for everybody
// TODO: Include optional [white]/[black], m, f in RegEx check

const char* boolPattern = "t|f";
const char* ratedPattern = "r|u";

const QString namePattern = QLatin1String ( "([a-zA-z\\(\\)]+)" );
const QString FicsProtocol::ratingPattern = QLatin1String ( "([0-9\\+\\-\\s]+)" );
const QString FicsProtocol::timePattern = QLatin1String ( "(\\d+)\\s+(\\d+)" );
const QString FicsProtocol::variantPattern = QLatin1String ( "([a-z]+)\\s+([a-z]+)" );
const QString FicsProtocol::argsPattern = QLatin1String ( "(.*)" ); //TODO better
const QString idPattern = QLatin1String ( "(\\d+)" );
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

const QRegExp seekExp ( QString ( QLatin1String("%1 w=%2 ti=%3 rt=%4  t=%5 i=%6 r=%7 tp=%8 c=%9 rr=%10 a=%11 f=%12") )
        .arg ( idPattern ) // %1 = index
        .arg ( namePattern ) // %2 = name
        .arg ( QLatin1String("0x([0-9a-f]{2,2})") ) // %3 = titles
        .arg ( QLatin1String("(\\d+)") ) // %4 = rating
        .arg ( QLatin1String("(\\d+)") ) // %5 = time
        .arg ( QLatin1String("(\\d+)") ) // %6 = increment
        .arg ( QLatin1String(ratedPattern) ) // %7 = rated ('r' or 'u')
        .arg ( QLatin1String("([a-z]+)") ) // %8 = type (standard, blitz, lightning, etc.)
        .arg ( QLatin1String("([?WB])") ) // %9 = color ('?', 'W' or 'B')
        .arg ( QLatin1String("([\\d+])\\-([\\d+])") ) // %10 = rating range (x-y)
        .arg ( QLatin1String(boolPattern) ) // %11 = automatic ( 't' or 'f' )
        .arg ( QLatin1String(boolPattern) ) // %12 = formula ( 't' or 'f' )
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

FicsProtocol::FicsProtocol ( QObject* parent ) : Protocol ( parent ),
    sendPassword(false),
    m_widget(0),
    m_terminal(0),
    konsoleFilter(0)
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
    return TimeLimit | SetTimeLimit | UpdateTime | Adjourn | Resign;
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


    // FICS Console
    KPluginLoader loader( QLatin1String( "libkonsolepart" ) );
    m_part = loader.factory()->create<KParts::ReadOnlyPart>(this);
    if (m_part)
    {
        m_terminal = qobject_cast< TerminalInterfaceV2* >( m_part );
        konsoleFilter = new KeyboardEventFilter(this);
        m_part->widget()->installEventFilter(konsoleFilter);
        connect ( konsoleFilter, SIGNAL(textTyped(QString)), this, SLOT(writeToSocket(QString)));
        connect(konsoleFilter, SIGNAL(enterPressed()), SLOT(flushSocket()));
    }

    m_socket = new QTcpSocket ( this );
    m_stream.setDevice ( m_socket );
    QString address = options.value ( QLatin1String ( "address" ), QLatin1String ( "freechess.org" ) ).toString();
    int port = options.value ( QLatin1String ( "port" ), 23 ).toInt();
    connect ( m_socket, SIGNAL ( connected() ), SLOT ( socketConnected() ) );
    connect ( m_socket, SIGNAL ( error ( QAbstractSocket::SocketError ) ), SLOT ( socketError() ) );
    connect ( m_socket, SIGNAL ( readyRead() ), SLOT ( readFromSocket() ) );
    m_socket->connectToHost ( address, port );
}

QWidgetList FicsProtocol::toolWidgets()
{
    QWidgetList widgets;
    widgets << m_part->widget();

    // FICS Chat Widget
    // TODO

    return widgets;
}

void FicsProtocol::socketConnected()
{
    kDebug();
}

void FicsProtocol::socketError()
{
    kDebug() << m_socket->errorString();
}

void FicsProtocol::login ( const QString& username, const QString& password )
{
    setPlayerName(username);
    m_stream << username << endl;
    sendPassword = true;
    this->password = password;
}

void FicsProtocol::setupOptions()
{
    m_stream << "set style 12" << endl;
    m_stream << "iset seekremove 1" << endl;
    m_stream << "iset seekinfo 1" << endl;
    m_stream << "set seek 1" << endl;
}

void FicsProtocol::openGameDialog()
{
    if ( m_widget )
    {
        m_widget->setStatus(i18n("Login failed"), true);
        return;
    }
    KDialog* dialog = new KDialog ( qApp->activeWindow() );
    dialog->setCaption(i18n("Chess server"));
    dialog->setButtons ( KDialog::Yes | KDialog::No | KDialog::Cancel | KDialog::User1 );

    dialog->setButtonText(KDialog::No, i18n("Decline"));
    dialog->button(KDialog::No)->setVisible(false);
    dialog->setButtonText(KDialog::Yes, i18n("Accept"));
    dialog->button(KDialog::Yes)->setVisible(false);
    dialog->setButtonText(KDialog::User1, i18n("Refresh"));
    dialog->setButtonIcon(KDialog::User1, KIcon(QLatin1String("view-refresh")));
    dialog->button(KDialog::User1)->setVisible(false);

    m_widget = new FicsDialog ( dialog );
    m_widget->setServerName(m_socket->peerName());
    m_widget->setConsoleWidget(m_part->widget());
    dialog->setMainWidget ( m_widget );

    connect ( dialog, SIGNAL ( applyClicked() ), m_widget, SLOT ( slotLogin()) );
    connect ( dialog, SIGNAL ( resetClicked() ), m_widget, SLOT ( decline() ) );
    connect ( dialog, SIGNAL(user1Clicked()), m_widget, SLOT(refresh()));
    connect ( m_widget, SIGNAL ( acceptSeek ( int ) ), SLOT ( acceptSeek ( int ) ) );
    connect ( m_widget, SIGNAL ( acceptChallenge() ), SLOT ( acceptChallenge() ) );
    connect ( m_widget, SIGNAL ( declineChallenge() ), SLOT ( declineChallenge() ) );
    connect ( m_widget, SIGNAL ( acceptButtonNeeded ( bool ) ), dialog->button ( KDialog::Yes ), SLOT ( setVisible(bool)) );
    connect ( m_widget, SIGNAL ( declineButtonNeeded ( bool ) ), dialog->button ( KDialog::No ), SLOT ( setVisible(bool)) );
    connect ( m_widget, SIGNAL ( reloadButtonNeeded ( bool ) ), dialog->button ( KDialog::User1 ), SLOT ( setVisible(bool)) );

    connect ( m_widget, SIGNAL(login(QString,QString)), this, SLOT(login(QString,QString)));
    
    connect ( this, SIGNAL(sessionStarted()), m_widget, SLOT(slotSessionStarted() ) );
    connect ( this, SIGNAL ( gameOfferReceived ( FicsGameOffer ) ), m_widget, SLOT ( addGameOffer ( FicsGameOffer ) ) );
    connect ( this, SIGNAL ( challengeReceived ( FicsPlayer ) ), m_widget, SLOT ( addChallenge ( FicsPlayer ) ) );
    connect ( m_widget, SIGNAL ( sought() ), SLOT ( checkSought() ) );
    connect ( m_widget, SIGNAL ( seekingChanged ( bool ) ), SLOT ( setSeeking ( bool ) ) );

    // connect ( dialog, SIGNAL(accepted()), SLOT(dialogAccepted()));
    connect ( dialog, SIGNAL ( rejected() ), SLOT ( dialogRejected() ) );

    connect ( this, SIGNAL ( initSuccesful() ), dialog, SLOT ( accept() ) );
    connect ( this, SIGNAL ( error ( Protocol::ErrorCode, QString ) ), dialog, SLOT ( deleteLater() ) );
    if ( Settings::autoLogin() )
    {
        m_widget->slotLogin();
    }
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
    if ( line.isEmpty() || line.contains("fics%") )
    {
        return;
    }
    line.chop(1); // Remove the extra newline
    if ( m_terminal )
    {
        m_terminal->sendInput( QLatin1String(line) );
    }
    kDebug() << "Read line:" << line;
    switch ( m_stage )
    {
        case ConnectStage:
            if ( line.contains ( "login:" ) )
            {
                openGameDialog();
            }
            else if ( line.contains ( "password:" ) )
            {
                if ( sendPassword )
                {
                    m_stream << password << endl;
                }
                else
                {
                    konsoleFilter->setPasswordMode(true);
                }
            }
            else if ( line.contains ( "Press return to enter the server" ) )
            {
                m_stream << endl;
            }
            // TODO: Check for incorrect logins
            else if ( line.contains ( "Starting FICS session" ) )
            {
                kDebug() << "Starting session" << line;
                m_stage = SeekStage;
                konsoleFilter->setPasswordMode(false);
                setupOptions();
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
                kDebug() << "Your name is" << name;
                setPlayerName ( name );
                emit sessionStarted();
            }
            else if ( line.contains ( "Invalid password" ) )
            {
                m_widget->setStatus(i18n("Invalid Password"), true);
            }
            break;
        case SeekStage:
            if ( line.startsWith("<sc>") )
            {
                emit clearSeeks();
            }
            else if ( line.startsWith("<sr>") )
            {
                foreach ( const QByteArray& str, line.split(' ') )
                {
                    bool ok;
                    int id = str.toInt(&ok);
                    if ( ok )
                    {
                        emit gameOfferRemoved(id);
                    }
                }
            }
            else if ( line.startsWith("<s>") && seekExp.indexIn(QLatin1String(line)) > -1 )
            {
                FicsGameOffer offer;
                int n = 1;
                offer.gameId = seekExp.cap(n++).toInt();
                offer.player.first = seekExp.cap(n++);
                n++; // Ignore titles for now, TODO
                offer.player.second = seekExp.cap(n++).toInt();
                offer.baseTime = seekExp.cap(n++).toInt();
                offer.timeIncrement = seekExp.cap(n++).toInt();
                offer.rated = ( !seekExp.cap(n).isEmpty() && seekExp.cap(n++)[0] == QLatin1Char('r') );
                offer.variant = seekExp.cap(n++);
                offer.color = parseColor(seekExp.cap(n++));
                offer.ratingRange.first = seekExp.cap(n++).toInt();
                offer.ratingRange.second = seekExp.cap(n++).toInt();
                offer.automatic = ( !seekExp.cap(n).isEmpty() && seekExp.cap(n++)[0] == QLatin1Char('t') );
                offer.formula = ( !seekExp.cap(n).isEmpty() && seekExp.cap(n++)[0] == QLatin1Char('t') );
            }
            else if ( seekRegExp.indexIn ( QLatin1String ( line ) ) != -1 )
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

                kDebug() << "Move:" << moveString;

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

Color FicsProtocol::parseColor ( QString str )
{
    if ( str.isEmpty() || str[0] == QLatin1Char('?') )
    {
        return NoColor;
    }
    if ( str[0] == QLatin1Char('W') )
    {
        return White;
    }
    if ( str[0] == QLatin1Char('B') )
    {
        return Black;
    }
    return NoColor;
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

void FicsProtocol::writeToSocket ( const QString& text )
{
    kDebug() << text;
    m_stream << text;
}

void FicsProtocol::flushSocket()
{
    kDebug();
    m_stream << endl;
}

void FicsProtocol::adjourn()
{
    m_stream << "adjourn" << endl;
}

void FicsProtocol::resign()
{
    m_stream << "resign" << endl;
}





// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
