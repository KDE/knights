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

#include "proto/ficsprotocol.h"
#include "proto/ficsdialog.h"
#include "proto/chatwidget.h"
#include "proto/keyboardeventfilter.h"
#include "settings.h"

#include <KDialog>
#include <KLocale>
#include <KPushButton>

#include <QtNetwork/QTcpSocket>
#include <QtGui/QApplication>
#include <QtCore/QPointer>
#include <gamemanager.h>

using namespace Knights;

const int Timeout = 1000; // One second ought to be enough for everybody
// TODO: Include optional [white]/[black], m, f in RegEx check

const char* boolPattern = "([tf])";
const char* ratedPattern = "([ru])";
const char*  namePattern = "([a-zA-z\\(\\)]+)";
const char*  ratingPattern = "([0-9\\+\\-\\s]+)";
const char* timePattern = "(\\d+)\\s+(\\d+)";
const char* variantPattern = "([a-z]+)\\s+([a-z]+)";
const char* argsPattern = "(.*)"; //TODO better
const char*  idPattern = "(\\d+)";
const char* pieces = "PRNBQKprnbqk";
const char* coordinate = "[abdcdefgh][12345678]";
const char* remainingTime = "\\d+ \\d+ (\\d+) \\d+ \\d+ (\\d+) (\\d+) (\\d+)";
const char* currentPlayerPattern = "([WB]) \\-?\\d+ \\d+ \\d+ \\d+ \\d+ \\d+ \\d+";

FicsProtocol::FicsProtocol ( QObject* parent ) : TextProtocol ( parent ),
    movePattern(QString(QLatin1String("(none|o-o|o-o-o|[%1]\\/%2\\-%3(=[%4])?)"))
        .arg ( QLatin1String(pieces) )
        .arg ( QLatin1String(coordinate) )
        .arg ( QLatin1String(coordinate) )
        .arg ( QLatin1String(pieces) ) ),
    seekExp ( QString ( QLatin1String("%1 w=%2 ti=%3 rt=%4[PE\\s] t=%5 i=%6 r=%7 tp=%8 c=%9 rr=%10 a=%11 f=%12") )
        .arg ( QLatin1String(idPattern) ) // %1 = index
        .arg ( QLatin1String(namePattern) ) // %2 = name
        .arg ( QLatin1String("([0-9a-f]{2,2})") ) // %3 = titles
        .arg ( QLatin1String("(\\d+)") ) // %4 = rating
        .arg ( QLatin1String("(\\d+)") ) // %5 = time
        .arg ( QLatin1String("(\\d+)") ) // %6 = increment
        .arg ( QLatin1String(ratedPattern) ) // %7 = rated ('r' or 'u')
        .arg ( QLatin1String("([a-z]+)") ) // %8 = type (standard, blitz, lightning, etc.)
        .arg ( QLatin1String("([?WB])") ) // %9 = color ('?', 'W' or 'B')
        .arg ( QLatin1String("(\\d+)\\-(\\d+)") ) // %10 = rating range (x-y)
        .arg ( QLatin1String(boolPattern) ) // %11 = automatic ( 't' or 'f' )
        .arg ( QLatin1String(boolPattern) )), // %12 = formula ( 't' or 'f' )
    challengeExp(QString ( QLatin1String("%1 w=%2 t=match p=%2 \\(%3\\) %2 \\(%3\\)"))
        .arg ( QLatin1String( idPattern ) ) // %1 = index
        .arg ( QLatin1String( namePattern ) ) // %2 = name
        .arg ( QLatin1String( ratingPattern ) ) ), // %3 = rating
    moveStringExp ( movePattern ),
    moveRegExp ( QString ( QLatin1String("<12>.*%1.*%2 %3") )
        .arg ( QLatin1String( currentPlayerPattern ) )
        .arg ( QLatin1String( remainingTime ) )
        .arg ( movePattern ) ),
    gameStartedExp ( QString ( QLatin1String("Creating: %1 \\(%2\\) %3 \\(%4\\) %5 %6") )
        .arg ( QLatin1String( namePattern ) )
        .arg ( QLatin1String( ratingPattern ) )
        .arg ( QLatin1String( namePattern ) )
        .arg ( QLatin1String( ratingPattern ) )
        .arg ( QLatin1String( variantPattern ) )
        .arg ( QLatin1String( timePattern ) ) ),
    sendPassword(false),
    m_widget(0),
    m_console(0),
    m_chat(0)
{
    // FICS games are always time-limited
    setAttribute ( QLatin1String("TimeLimitEnabled"), true );
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
    write(m.string(false));
}

void FicsProtocol::init ( const QVariantMap& options )
{
    setAttributes ( options );
    m_stage = ConnectStage;

    m_console = createConsoleWidget();
    m_console->addExtraButton ( QLatin1String("seek"), i18n("Seek"), QLatin1String("edit-find") );
    m_console->addExtraButton ( QLatin1String("unseek"), i18n("Unseek"), QLatin1String("edit-clear") );
    m_console->addExtraButton ( QLatin1String("accept"), i18n("Accept"), QLatin1String("dialog-ok-accept") );
    m_console->addExtraButton ( QLatin1String("help"), i18n("Help"), QLatin1String("help-contents") );
    connect ( m_console, SIGNAL(sendText(QString)), SLOT(writeCheckMoves(QString)) );

    QTcpSocket* socket = new QTcpSocket ( this );
    setDevice ( socket );
    QString address = options.value ( QLatin1String("address"), QLatin1String("freechess.org") ).toString();
    int port = options.value ( QLatin1String("port"), 5000 ).toInt();
    connect ( socket, SIGNAL ( error ( QAbstractSocket::SocketError ) ), SLOT ( socketError() ) );
    socket->connectToHost ( address, port );
}

QList< Protocol::ToolWidgetData > FicsProtocol::toolWidgets()
{
    ToolWidgetData consoleData;
    consoleData.widget = m_console;
    consoleData.title = i18n("Server Console");
    consoleData.name = QLatin1String("console");

    if ( !m_chat )
    {
        m_chat = createChatWidget();
        connect ( m_chat, SIGNAL(sendText(QString)), SLOT(sendChat(QString)));
    }

    ToolWidgetData chatData;
    chatData.widget = m_chat;
    chatData.title = i18n("Chat with %1", playerName());
    chatData.name = QLatin1String("chat");

    return QList<ToolWidgetData>() << consoleData << chatData;
}

void FicsProtocol::socketError()
{
    emit error( NetworkError, QString() );
}

void FicsProtocol::login ( const QString& username, const QString& password )
{
    setPlayerName(username);
    write(username);
    sendPassword = true;
    this->password = password;
}

void FicsProtocol::setupOptions()
{
    write("set style 12");
    write("iset seekremove 1");
    write("iset seekinfo 1");
    write("iset pendinfo 1");
    write("set seek 1");
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
    dialog->setButtons ( KDialog::User1 | KDialog::User2 | KDialog::Cancel );

    dialog->setButtonText(KDialog::User2, i18n("Decline"));
    dialog->setButtonIcon(KDialog::User2, KIcon(QLatin1String("dialog-close")));
    dialog->button(KDialog::User2)->setVisible(false);
    
    dialog->setButtonText(KDialog::User1, i18n("Accept"));
    dialog->button(KDialog::User1)->setVisible(false);
    dialog->setButtonIcon(KDialog::User1, KIcon(QLatin1String("dialog-ok-accept")));

    m_widget = new FicsDialog ( dialog );
    m_widget->setServerName ( attribute( "address" ).toString());
    m_widget->setConsoleWidget(m_console);
    dialog->setMainWidget ( m_widget );

    connect ( dialog, SIGNAL ( user2Clicked()), m_widget, SLOT(decline()) );
    connect ( dialog, SIGNAL ( user1Clicked()), m_widget, SLOT(accept()) );
    connect ( m_widget, SIGNAL ( acceptButtonNeeded ( bool ) ), dialog->button ( KDialog::User1 ), SLOT ( setVisible(bool)) );
    connect ( m_widget, SIGNAL ( declineButtonNeeded ( bool ) ), dialog->button ( KDialog::User2 ), SLOT ( setVisible(bool)) );

    connect ( m_widget, SIGNAL(login(QString,QString)), this, SLOT(login(QString,QString)));
    connect ( m_widget, SIGNAL(acceptSeek(int)), SLOT(acceptSeek(int)) );
    connect ( m_widget, SIGNAL(acceptChallenge(int)), SLOT(acceptChallenge(int)) );
    connect ( m_widget, SIGNAL(declineChallenge(int)), SLOT(declineChallenge(int)) );
    
    connect ( this, SIGNAL(sessionStarted()), m_widget, SLOT(slotSessionStarted() ) );
    connect ( this, SIGNAL ( gameOfferReceived ( FicsGameOffer ) ), m_widget, SLOT ( addGameOffer ( FicsGameOffer ) ) );
    connect ( this, SIGNAL ( gameOfferRemoved(int)), m_widget, SLOT ( removeGameOffer(int)) );
    connect ( this, SIGNAL ( challengeReceived ( FicsChallenge ) ), m_widget, SLOT ( addChallenge ( FicsChallenge ) ) );
    connect ( this, SIGNAL(gameOfferRemoved(int)), m_widget, SLOT(removeChallenge(int)) );
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

bool FicsProtocol::parseStub(const QString& line)
{
    if ( line.contains ( QLatin1String("fics") ) && line.contains ( QLatin1String("login:") ) && line.contains ( QLatin1String("password:") ) )
    {
        parseLine(line);
        return true;
    }
    return false;
}

void FicsProtocol::parseLine(const QString& line)
{
    if ( line.isEmpty() || line.startsWith( QLatin1String("fics%") ) )
    {
        return;
    }
    bool display = true;
    ChatWidget::MessageType type = ChatWidget::GeneralMessage;
    switch ( m_stage )
    {
        case ConnectStage:
            if ( line.contains ( QLatin1String("login:") ) )
            {
                type = ChatWidget::AccountMessage;
                openGameDialog();
            }
            else if ( line.contains ( QLatin1String("password:") ) )
            {
                type = ChatWidget::AccountMessage;
                if ( sendPassword )
                {
                    write(password);
                }
                else
                {
                    m_console->setPasswordMode(true);
                }
            }
            else if ( line.contains ( QLatin1String("Press return to enter the server") ) )
            {
                type = ChatWidget::AccountMessage;
                write(QString());
            }
            // TODO: Check for incorrect logins
            else if ( line.contains ( QLatin1String("Starting FICS session") ) )
            {
                type = ChatWidget::StatusMessage;
                m_stage = SeekStage;
                m_console->setPasswordMode(false);
                setupOptions();
                QString name = line;
                name.remove ( 0, name.indexOf ( QLatin1String("session as ") ) + 11 );
                if ( name.contains ( QLatin1String("(U)") ) )
                {
                    name.truncate ( name.indexOf ( QLatin1String("(U)") ) );
                }
                else
                {
                    name.truncate ( name.indexOf ( QLatin1Char ( ' ' ) ) );
                }
                kDebug() << QLatin1String("Your name is") << name;
                setPlayerName ( name );
                emit sessionStarted();
            }
            else if ( line.contains ( QLatin1String("Invalid password") ) )
            {
                m_widget->setLoginEnabled ( true );
                type = ChatWidget::AccountMessage;
                m_widget->setStatus(i18n("Invalid Password"), true);
            }
            break;
        case SeekStage:
            if ( line.startsWith( QLatin1String("<sc>") ) )
            {
                display = false;
                emit clearSeeks();
            }
            else if ( line.startsWith( QLatin1String("<sr>") ) )
            {
                display = false;
                foreach ( const QString& str, line.split(QLatin1Char(' ') ) )
                {
                    bool ok;
                    int id = str.toInt(&ok);
                    if ( ok )
                    {
                        emit gameOfferRemoved(id);
                    }
                }
            }
            else if ( line.startsWith( QLatin1String("<s>") ) && seekExp.indexIn(line) > -1 )
            {
                display = false;
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
                emit gameOfferReceived ( offer );
            }
            else if ( line.startsWith( QLatin1String("<pf>") ) && challengeExp.indexIn ( line ) > -1 )
            {
                display = false;
                FicsChallenge challenge;
                challenge.gameId = challengeExp.cap ( 1 ).toInt();
                challenge.player.first = challengeExp.cap ( 2 );
                int ratingPos = ( challengeExp.cap(1) == challengeExp.cap(3) ) ? 4 : 6;
                challenge.player.second = challengeExp.cap ( ratingPos ).toInt();
                emit challengeReceived ( challenge );
            }
            else if ( line.startsWith( QLatin1String("<pr>") ) )
            {
                display = false;
                foreach ( const QString& str, line.split( QLatin1Char(' ') ) )
                {
                    bool ok;
                    int id = str.toInt(&ok);
                    if ( ok )
                    {
                        emit challengeRemoved(id);
                    }
                }
            }
            else if ( gameStartedExp.indexIn ( line ) > -1 )
            {
                type = ChatWidget::StatusMessage;
                QString player1 = gameStartedExp.cap ( 1 );
                QString player2 = gameStartedExp.cap ( 3 );
                if ( player1 == playerName() )
                {
                    setColor ( Black );
                    setPlayerName ( player2 );
                }
                else
                {
                    setColor ( White );
                    setPlayerName ( player1 );
                }
                m_stage = PlayStage;
                emit initSuccesful();
            }
            break;
        case PlayStage:
            if ( moveRegExp.indexIn ( line ) > -1 )
            {
                display = false;
                bool validMove = !( moveRegExp.cap ( 1 ) == QLatin1String("B") && color() == White )
                        && !( moveRegExp.cap ( 1 ) == QLatin1String("W") && color() == Black );

                const int whiteTimeLimit = moveRegExp.cap ( 3 ).toInt();
                const int blackTimeLimit = moveRegExp.cap ( 4 ).toInt();
                const QString  moveString = moveRegExp.cap ( 6 );

                kDebug() << "Move:" << moveString;

                if ( moveString == QLatin1String("none") )
                {
                    TimeControl tc;
                    tc.moves = 0;
                    tc.baseTime = QTime().addSecs(whiteTimeLimit);
                    tc.increment = moveRegExp.cap(2).toInt();
                    manager->setTimeControl(NoColor, tc);
                    break;
                }

                if ( validMove )
                {
                    Move m;
                    if ( moveString == QLatin1String("o-o") )
                    {
                        // Short (king's rook) castling
                        m = Move::castling ( Move::KingSide, color() );
                    }
                    else if ( moveString == QLatin1String("o-o-o") )
                    {
                        // Long (Queen's rock) castling
                        m = Move::castling ( Move::QueenSide, color() );
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
                    emit pieceMoved ( m );
                    manager->changeActivePlayer();
                }
                manager->setCurrentTime ( White, QTime().addSecs ( whiteTimeLimit ) );
                manager->setCurrentTime ( Black, QTime().addSecs ( blackTimeLimit ) );

                if ( moveRegExp.cap(5).toInt() == 2 )
                {
                    manager->startTime();
                }
            }
            else if ( line.contains ( QLatin1String(") says:") ) )
            {
                type = ChatWidget::ChatMessage;
                m_chat->addText ( line, type );
            }
            else if ( line.contains ( QLatin1String("lost contact or quit") ) )
            {
                type = ChatWidget::AccountMessage;
                emit gameOver ( NoColor );
            }
    }

    if ( display )
    {
        m_console->addText( line, type );
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

void FicsProtocol::acceptSeek ( int id )
{
    write ( QLatin1String("play ") + QString::number(id) );
    m_seeking = false;
}

void FicsProtocol::acceptChallenge ( int id )
{
    write ( QLatin1String("accept ") + QString::number(id) );
    m_seeking = true;
}

void FicsProtocol::declineChallenge ( int id )
{
    write ( QLatin1String("decline ") + QString::number(id) );
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
        write("seek");
        QString seekStr;
        if ( attribute ( QLatin1String("playerTimeLimit") ).canConvert<QTime>() && attribute ( QLatin1String("playerTimeIncrement") ).canConvert<int>() )
        {
            QTime time = attribute ( QLatin1String("playerTimeLimit") ).toTime();
            int increment = attribute ( QLatin1String("playerTimeIncrement") ).toInt();
            seekStr += ( QLatin1Char(' ')
                    + QString::number(60 * time.hour() + time.minute())
                    + QLatin1Char(' ')
                    + QString::number(increment) );
        }
        seekStr += QLatin1String(" unrated");
        switch ( oppositeColor( color() ) )
        {
            case White:
                seekStr += QLatin1String(" white");
                break;
            case Black:
                seekStr += QLatin1String(" black");
                break;
            default:
                break;
        }
        seekStr += QLatin1String(" manual");
        write(seekStr);
    }
    else
    {
        write("unseek");
    }
}


void FicsProtocol::adjourn()
{
    write("adjourn");
}

void FicsProtocol::resign()
{
    write("resign");
}

void FicsProtocol::sendChat ( QString text )
{
    write ( QLatin1String("say ") + text );
}





// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
