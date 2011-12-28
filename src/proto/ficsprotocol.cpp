/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>

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
#include "settings.h"

#include <KDialog>
#include <KLocale>
#include <KPushButton>

#include <QtNetwork/QTcpSocket>
#include <QtGui/QApplication>
#include <gamemanager.h>

using namespace Knights;

const int Timeout = 1000; // One second ought to be enough for everybody
// TODO: Include optional [white]/[black], m, f in RegEx check

const char* boolPattern = "([tf])";
const char* ratedPattern = "([ru])";
const char*  namePattern = "([a-zA-Z\\(\\)]+)";
const char*  ratingPattern = "([0-9\\+\\-\\s]+)";
const char* timePattern = "(\\d+)\\s+(\\d+)";
const char* variantPattern = "([a-z]+)\\s+([a-z]+)";
const char* argsPattern = "(.*)"; //TODO better
const char*  idPattern = "(\\d+)";
const char* pieces = "PRNBQKprnbqk";
const char* coordinate = "[abdcdefgh][12345678]";
const char* remainingTime = "\\d+ \\d+ (\\d+) \\d+ \\d+ (\\d+) (\\d+) (\\d+)";
const char* currentPlayerPattern = "([WB]) \\-?\\d+ \\d+ \\d+ \\d+ \\d+ \\d+ \\d+";
const char* offerPattern = "<pf> (\\d+) w=([a-zA-Z\\(\\)]+) t=([a-z]+) p=(.+)";

FicsProtocol::FicsProtocol ( QObject* parent ) : TextProtocol ( parent ),
    movePattern(QString(QLatin1String("[%1]\\/(%2)\\-(%3)(=[%4])?"))
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
    moveRegExp ( QString ( QLatin1String("<12>.*%1.*%2 (none|o-o|o-o-o|%3)") )
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
    offerExp ( QLatin1String( offerPattern ) ),
    sendPassword(false),
    m_widget(0),
    m_chat(0)
{
    // FICS games are always time-limited
    setAttribute ( QLatin1String("TimeLimitEnabled"), true );
    if ( !Manager::self()->timeControlEnabled(color()) )
    {
        TimeControl tc;
        tc.baseTime = QTime().addSecs( 10 * 60 ); // A default time of 10 minutes with no increment
        tc.increment = 0;
        Manager::self()->setTimeControl(color(), tc);
    }
}

FicsProtocol::~FicsProtocol()
{

}

Protocol::Features FicsProtocol::supportedFeatures()
{
    return TimeLimit | SetTimeLimit | UpdateTime | Pause | Adjourn | Resign | Abort;
}

void FicsProtocol::startGame()
{

}

void FicsProtocol::move ( const Move& m )
{
    write(m.string(false));
}

void FicsProtocol::init (  )
{
    m_stage = ConnectStage;

    ChatWidget* console = createConsoleWidget();
    console->addExtraButton ( QLatin1String("seek"), i18nc("Start searching for opponents", "Seek"), QLatin1String("edit-find") );
    console->addExtraButton ( QLatin1String("unseek"), i18nc("Stop searching for opponents", "Unseek"), QLatin1String("edit-clear") );
    console->addExtraButton ( QLatin1String("accept"), i18n("Accept"), QLatin1String("dialog-ok-accept") );
    console->addExtraButton ( QLatin1String("help"), i18n("Help"), QLatin1String("help-contents") );
    connect ( console, SIGNAL(sendText(QString)), SLOT(writeCheckMoves(QString)) );
    setConsole ( console );

    QTcpSocket* socket = new QTcpSocket ( this );
    setDevice ( socket );
    QString address = attribute("server").toString();
    int port = attribute("port").toInt();
    if ( port == 0 )
    {
        port = 5000;
    }
    connect ( socket, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(socketError()) );
    socket->connectToHost ( address, port );
}

QList< Protocol::ToolWidgetData > FicsProtocol::toolWidgets()
{
    ToolWidgetData consoleData;
    consoleData.widget = console();
    consoleData.title = i18n("Server Console");
    consoleData.name = QLatin1String("console");
    consoleData.type = ConsoleToolWidget;
    consoleData.owner = color();

    if ( !m_chat )
    {
        m_chat = createChatWidget();
        connect ( m_chat, SIGNAL(sendText(QString)), SLOT(sendChat(QString)));
    }

    ToolWidgetData chatData;
    chatData.widget = m_chat;
    chatData.title = i18n("Chat with %1", playerName());
    chatData.name = QLatin1String("chat");
    chatData.type = ChatToolWidget;

    return QList<ToolWidgetData>() << consoleData << chatData;
}

void FicsProtocol::socketError()
{
    emit error( NetworkError, device()->errorString() );
}

void FicsProtocol::login ( const QString& username, const QString& password )
{
    otherPlayerName = username;
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
        Settings::setAutoLogin(false);
        m_widget->setLoginEnabled(true);
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
    m_widget->setServerName ( attribute( "server" ).toString());
    m_widget->setConsoleWidget ( console() );
    dialog->setMainWidget ( m_widget );

    connect ( dialog, SIGNAL (user2Clicked()), m_widget, SLOT(decline()) );
    connect ( dialog, SIGNAL (user1Clicked()), m_widget, SLOT(accept()) );
    connect ( m_widget, SIGNAL (acceptButtonNeeded(bool)), dialog->button ( KDialog::User1 ), SLOT (setVisible(bool)) );
    connect ( m_widget, SIGNAL (declineButtonNeeded(bool)), dialog->button ( KDialog::User2 ), SLOT (setVisible(bool)) );

    connect ( m_widget, SIGNAL(login(QString,QString)), this, SLOT(login(QString,QString)));
    connect ( m_widget, SIGNAL(acceptSeek(int)), SLOT(acceptSeek(int)) );
    connect ( m_widget, SIGNAL(acceptChallenge(int)), SLOT(acceptChallenge(int)) );
    connect ( m_widget, SIGNAL(declineChallenge(int)), SLOT(declineChallenge(int)) );
    
    connect ( this, SIGNAL(sessionStarted()), m_widget, SLOT(slotSessionStarted()) );
    connect ( this, SIGNAL (gameOfferReceived(FicsGameOffer)), m_widget, SLOT (addGameOffer(FicsGameOffer)) );
    connect ( this, SIGNAL (gameOfferRemoved(int)), m_widget, SLOT (removeGameOffer(int)) );
    connect ( this, SIGNAL (challengeReceived(FicsChallenge)), m_widget, SLOT (addChallenge(FicsChallenge)) );
    connect ( this, SIGNAL(gameOfferRemoved(int)), m_widget, SLOT(removeChallenge(int)) );
    connect ( m_widget, SIGNAL (seekingChanged(bool)), SLOT (setSeeking(bool)) );

    // connect ( dialog, SIGNAL(accepted()), SLOT(dialogAccepted()));
    connect ( dialog, SIGNAL (rejected()), SLOT (dialogRejected()) );

    connect ( this, SIGNAL (initSuccesful()), dialog, SLOT (accept()) );
    connect ( this, SIGNAL(initSuccesful()), m_widget, SLOT(slotDialogAccepted()) );
    connect ( this, SIGNAL (error(Protocol::ErrorCode,QString)), dialog, SLOT (deleteLater()) );
    if ( Settings::autoLogin() )
    {
        m_widget->slotLogin();
    }
    dialog->show();
}

bool FicsProtocol::parseStub(const QString& line)
{
    Q_UNUSED(line);
    return false;
}

bool FicsProtocol::parseLine(const QString& line)
{
    if ( line.isEmpty() || line.startsWith( QLatin1String("fics%") ) )
    {
        return true;
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
                    console()->setPasswordMode(true);
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
                console()->setPasswordMode(false);
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
                otherPlayerName = name;
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
                int ratingPos = ( challengeExp.cap(2) == challengeExp.cap(3) ) ? 4 : 6;
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
                kDebug() << "Game Started" << line;
                type = ChatWidget::StatusMessage;
                QString player1 = gameStartedExp.cap ( 1 );
                QString player2 = gameStartedExp.cap ( 3 );
                Color color = NoColor;
                if ( player1 == otherPlayerName )
                {
                    color = Black;
                    setPlayerName ( player2 );
                }
                else
                {
                    color = White;
                    setPlayerName ( player1 );
                }
                if ( byColor(color) != this )
                {
                    kDebug() << "Switching protocols";
                    // The color is different than was assigned at first
                    // We have to switch the protocols
                    Protocol* t = white();
                    setWhiteProtocol(black());
                    setBlackProtocol(t);
                }
                Protocol* opp = Protocol::byColor ( oppositeColor ( color ) );
                if ( opp->isLocal() )
                {
                    opp->setPlayerName ( otherPlayerName );
                }
                
                m_stage = PlayStage;
                initComplete();
            }
            break;
        case PlayStage:
            if ( moveRegExp.indexIn ( line ) > -1 )
            {
                display = false;
                kDebug() << moveRegExp.cap(1) << colorName(color());
                bool validMove = !( moveRegExp.cap ( 1 ) == QLatin1String("W") && color() == White )
                        && !( moveRegExp.cap ( 1 ) == QLatin1String("B") && color() == Black );

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
                    Manager::self()->setTimeControl(NoColor, tc);
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
                        if ( !moveStringExp.cap(3).isEmpty() )
                        {
                            m.setFlag ( Move::Promote, true );
                            QChar typeChar = moveRegExp.cap ( 3 ).mid ( 1, 1 ) [0];
                            m.setPromotedType ( Piece::typeFromChar ( typeChar ) );
                        }
                    }
                    kDebug() << "Valid move" << m;
                    emit pieceMoved ( m );
                }
                Manager::self()->setCurrentTime ( White, QTime().addSecs ( whiteTimeLimit ) );
                Manager::self()->setCurrentTime ( Black, QTime().addSecs ( blackTimeLimit ) );

                if ( moveRegExp.cap(5).toInt() == 2 )
                {
                    // TODO: Notify the manager that time is starting now
                }
            }
            else if ( offerExp.indexIn(line) > -1 )
            {
                Offer offer;
                offer.id = offerExp.cap(1).toInt();
                offer.player = color();
                QString type = offerExp.cap(3);
                if ( type == QLatin1String("abort") )
                {
                    offer.action = ActionAbort;
                }
                else if ( type == QLatin1String("adjourn") )
                {
                    offer.action = ActionAdjourn;
                }
                else if ( type == QLatin1String("draw") )
                {
                    offer.action = ActionDraw;
                }
                else if ( type == QLatin1String("takeback") )
                {
                    offer.action = ActionUndo;
                    offer.numberOfMoves = offerExp.cap(4).toInt();
                }
                m_offers.insert ( offer.id, offer );
                Manager::self()->sendOffer ( offer );
            }
            else if ( line.contains ( QLatin1String(" says:") ) )
            {
                type = ChatWidget::ChatMessage;
                m_chat->addText ( line, type );
            }
            else if ( line.contains ( QLatin1String("lost contact or quit") ) )
            {
                type = ChatWidget::AccountMessage;
            }
            else if ( line.startsWith(QLatin1Char('{')) && line.contains(QLatin1Char('}')))
            {
                if ( line.endsWith ( QLatin1String("1-0") ) )
                {
                    type = ChatWidget::AccountMessage;
                    emit gameOver ( White );
                }
                else if ( line.endsWith ( QLatin1String("1/2-1/2") ) || line.endsWith ( QLatin1Char('*') ) )
                {
                    // Knights has no way of reporting aborted or unfinished games
                    // so we report aborted games as draws
                    type = ChatWidget::AccountMessage;
                    emit gameOver ( NoColor );
                }
                else if ( line.endsWith ( QLatin1String("0-1") ) )
                {
                    type = ChatWidget::AccountMessage;
                    emit gameOver ( Black );
                }
            }
    }

    if ( display )
    {
        writeToConsole ( line, type );
    }
    return true;
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
        QByteArray seekText = "seek";
        
        TimeControl tc = Manager::self()->timeControl(color());
        seekText += ' ';
        seekText += QString::number ( 60 * tc.baseTime.hour() + tc.baseTime.minute() ).toAscii();
        seekText += ' ';
        seekText += QString::number ( tc.increment ).toAscii();

        seekText += m_widget->rated() ? " rated" : " unrated";
        /*
         * Commented out until I figure out a simple way to determine if a color should be forced. 
        switch ( color() )
        {
            case White:
                seekText += " white";
                break;
            case Black:
                seekText += " black";
                break;
            default:
                break;
        }
        */
        seekText += m_widget->autoAcceptChallenge() ? " auto" : " manual";
        kDebug() << seekText;
        write(QLatin1String(seekText));
    }
    else
    {
        write("unseek");
    }
}

void FicsProtocol::resign()
{
    write("resign");
}

void FicsProtocol::sendChat ( QString text )
{
    write ( QLatin1String("say ") + text );
}

void FicsProtocol::acceptOffer(const Offer& offer)
{
    write ( QLatin1String("accept ") + QString::number(offer.id) );
}

void FicsProtocol::declineOffer(const Offer& offer)
{
    write ( QLatin1String("decline ") + QString::number(offer.id) );
}

void FicsProtocol::makeOffer(const Offer& offer)
{
    switch (offer.action)
    {
        case ActionDraw:
            write ( "draw" );
            break;
            
        case ActionPause:
            write ( "pause" );
            break;
            
        case ActionUndo:
            write ( QLatin1String("takeback ") + QString::number ( offer.numberOfMoves ) );
            break;
            
        case ActionResume:
            write ( "unpause" );
            break;
            
        case ActionAdjourn:
            write ( "adjourn" );
            break;
            
        case ActionAbort:
            write ( "abort" );
            break;
            
        default:
            break;
    }
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
