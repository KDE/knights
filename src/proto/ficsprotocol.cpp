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
#include <gamemanager.h>
#include "proto/ficsdialog.h"
#include "proto/chatwidget.h"
#include "settings.h"
#include "knightsdebug.h"

#include <KLocalizedString>

#include <QDialog>
#include <QDialogButtonBox>
#include <QVBoxLayout>
#include <QPushButton>
#include <QAbstractSocket>
#include <QTcpSocket>
#include <QApplication>

using namespace Knights;

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
	movePattern(QStringLiteral("[%1]\\/(%2)\\-(%3)(=[%4])?")
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
    m_stage(ConnectStage),
	sendPassword(false),
	m_widget(nullptr),
	m_seeking(false),
	m_chat(nullptr)
    {
	// FICS games are always time-limited
	setAttribute ( QStringLiteral("TimeLimitEnabled"), true );

    //TODO: this initialization is obsolete since the time control _should_ be set
    //when a new game is started and the values provided in the game dialog are used.
	if ( !Manager::self()->timeControlEnabled(color()) ) {
		TimeControl tc;
		tc.moves = 100;
		tc.baseTime = QTime().addSecs( 10 * 60 ); // A default time of 10 minutes with no increment
		tc.increment = 0;
		Manager::self()->setTimeControl(color(), tc);
	}
}

FicsProtocol::~FicsProtocol() = default;

Protocol::Features FicsProtocol::supportedFeatures() {
	return TimeLimit | SetTimeLimit | UpdateTime | Pause | Adjourn | Resign | Abort;
}

void FicsProtocol::startGame() {

}

void FicsProtocol::move ( const Move& m ) {
	write(m.string(false));
}

void FicsProtocol::init (  ) {
	m_stage = ConnectStage;

	ChatWidget* console = createConsoleWidget();
	console->addExtraButton ( QStringLiteral("seek"), i18nc("Start searching for opponents", "Seek"), QStringLiteral("edit-find") );
	console->addExtraButton ( QStringLiteral("unseek"), i18nc("Stop searching for opponents", "Unseek"), QStringLiteral("edit-clear") );
	console->addExtraButton ( QStringLiteral("accept"), i18n("Accept"), QStringLiteral("dialog-ok-accept") );
	console->addExtraButton ( QStringLiteral("help"), i18n("Help"), QStringLiteral("help-contents") );
	connect ( console, &ChatWidget::sendText, this, &FicsProtocol::writeCheckMoves );
	setConsole ( console );

	QTcpSocket* socket = new QTcpSocket ( this );
	setDevice ( socket );
	QString address = attribute("server").toString();
	int port = attribute("port").toInt();
	if ( port == 0 )
		port = 5000;
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
	connect ( socket, &QTcpSocket::errorOccurred,
#else
	connect ( socket, static_cast<void (QTcpSocket::*)(QAbstractSocket::SocketError)> (&QTcpSocket::error),
#endif
	          this, &FicsProtocol::socketError );

	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	socket->connectToHost(address, port);
	if (socket->waitForConnected(5000))
		QApplication::restoreOverrideCursor();
}

QList< Protocol::ToolWidgetData > FicsProtocol::toolWidgets() {
	ToolWidgetData consoleData;
	consoleData.widget = console();
	consoleData.title = i18n("Server Console");
	consoleData.name = QStringLiteral("console");
	consoleData.type = ConsoleToolWidget;
	consoleData.owner = color();

	if ( !m_chat ) {
		m_chat = createChatWidget();
		connect ( m_chat, &ChatWidget::sendText, this, &FicsProtocol::sendChat );
	}

	ToolWidgetData chatData;
	chatData.widget = m_chat;
	chatData.title = i18n("Chat with %1", playerName());
	chatData.name = QStringLiteral("chat");
	chatData.type = ChatToolWidget;

	return QList<ToolWidgetData>() << consoleData << chatData;
}

void FicsProtocol::socketError() {
	QApplication::restoreOverrideCursor();
	Q_EMIT error( NetworkError, device()->errorString() );
}

void FicsProtocol::login ( const QString& username, const QString& password ) {
	otherPlayerName = username;
	write(username);
	sendPassword = true;
	this->password = password;
}

void FicsProtocol::setupOptions() {
	write("set style 12");
	write("iset seekremove 1");
	write("iset seekinfo 1");
	write("iset pendinfo 1");
	write("set seek 1");
}

void FicsProtocol::openGameDialog() {
	if ( m_widget ) {
		m_widget->setStatus(i18n("Login failed"), true);
		Settings::setAutoLogin(false);
		m_widget->setLoginEnabled(true);
		return;
	}
	QDialog* dialog = new QDialog ( qApp->activeWindow() );
	dialog->setWindowTitle(i18n("Chess server"));
	auto mainLayout = new QVBoxLayout(dialog);
	dialog->setLayout(mainLayout);
	auto bBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel );

	bBox->button(QDialogButtonBox::Ok)->setText(i18n("Accept"));
	bBox->button(QDialogButtonBox::Ok)->setVisible(false);
	bBox->button(QDialogButtonBox::Ok)->setIcon(QIcon::fromTheme(QStringLiteral("dialog-ok-accept")));

	bBox->button(QDialogButtonBox::Cancel)->setText(i18n("Decline"));
	bBox->button(QDialogButtonBox::Cancel)->setVisible(false);
	bBox->button(QDialogButtonBox::Cancel)->setIcon(QIcon::fromTheme(QStringLiteral("dialog-close")));

	m_widget = new FicsDialog ();
	m_widget->setServerName ( attribute( "server" ).toString());
	m_widget->setConsoleWidget ( console() );
	mainLayout->addWidget(m_widget);
	mainLayout->addWidget(bBox);

	connect ( bBox, &QDialogButtonBox::rejected, m_widget, &FicsDialog::decline );
	connect ( bBox, &QDialogButtonBox::accepted, m_widget, &FicsDialog::accept );
	connect ( m_widget, &FicsDialog::acceptButtonNeeded, bBox->button(QDialogButtonBox::Ok), &QPushButton::setVisible );
	connect ( m_widget, &FicsDialog::declineButtonNeeded, bBox->button(QDialogButtonBox::Cancel), &QPushButton::setVisible );

	connect ( m_widget, &FicsDialog::login, this, &FicsProtocol::login );
	connect ( m_widget, &FicsDialog::acceptSeek, this, &FicsProtocol::acceptSeek );
	connect ( m_widget, &FicsDialog::acceptChallenge, this, &FicsProtocol::acceptChallenge );
	connect ( m_widget, &FicsDialog::declineChallenge, this, &FicsProtocol::declineChallenge );

	connect ( this, &FicsProtocol::sessionStarted, m_widget, &FicsDialog::slotSessionStarted );
	connect ( this, &FicsProtocol::gameOfferReceived, m_widget, &FicsDialog::addGameOffer );
	connect ( this, &FicsProtocol::gameOfferRemoved, m_widget, &FicsDialog::removeGameOffer );
	connect ( this, &FicsProtocol::challengeReceived, m_widget, &FicsDialog::addChallenge );
	connect ( this, &FicsProtocol::gameOfferRemoved, m_widget, &FicsDialog::removeChallenge );
	connect ( m_widget, &FicsDialog::seekingChanged, this, &FicsProtocol::setSeeking );

	// connect ( dialog, &QDialog::accepted, this, &FicsProtocol::dialogAccepted );
	connect ( dialog, &QDialog::rejected, this, &FicsProtocol::dialogRejected );

	connect ( this, &FicsProtocol::initSuccesful, dialog, &QDialog::accept );
	/* TODO: The SLOT slotDialogAccepted() is not implemented. Need to recheck the intention */
	//connect ( this, &FicsProtocol::initSuccesful, m_widget, &FicsDialog::slotDialogAccepted );
	connect ( this, &FicsProtocol::error, dialog, &QDialog::deleteLater );
	if ( Settings::autoLogin() )
		m_widget->slotLogin();
	dialog->show();
}

bool FicsProtocol::parseStub(const QString& line) {
	Q_UNUSED(line);
	return false;
}

bool FicsProtocol::parseLine(const QString& line) {
	if ( line.isEmpty() || line.startsWith( QLatin1String("fics%") ) )
		return true;
	bool display = true;
	ChatWidget::MessageType type = ChatWidget::GeneralMessage;
	switch ( m_stage ) {
	case ConnectStage:
		if ( line.contains ( QLatin1String("login:") ) ) {
			type = ChatWidget::AccountMessage;
			openGameDialog();
		} else if ( line.contains ( QLatin1String("password:") ) ) {
			type = ChatWidget::AccountMessage;
			if ( sendPassword )
				write(password);
			else
				console()->setPasswordMode(true);
		} else if ( line.contains ( QLatin1String("Press return to enter the server") ) ) {
			type = ChatWidget::AccountMessage;
			write(QString());
		}
		// TODO: Check for incorrect logins
		else if ( line.contains ( QLatin1String("Starting FICS session") ) ) {
			type = ChatWidget::StatusMessage;
			m_stage = SeekStage;
			console()->setPasswordMode(false);
			setupOptions();
			QString name = line;
			name.remove ( 0, name.indexOf ( QLatin1String("session as ") ) + 11 );
			if ( name.contains ( QLatin1String("(U)") ) )
				name.truncate ( name.indexOf ( QLatin1String("(U)") ) );
			else
				name.truncate ( name.indexOf ( QLatin1Char ( ' ' ) ) );
			qCDebug(LOG_KNIGHTS) << QLatin1String("Your name is") << name;
			otherPlayerName = name;
			Q_EMIT sessionStarted();
		} else if ( line.contains ( QLatin1String("Invalid password") ) ) {
			m_widget->setLoginEnabled ( true );
			type = ChatWidget::AccountMessage;
			m_widget->setStatus(i18n("Invalid Password"), true);
		}
		break;
	case SeekStage:
		if ( line.startsWith( QLatin1String("<sc>") ) ) {
			display = false;
			Q_EMIT clearSeeks();
		} else if ( line.startsWith( QLatin1String("<sr>") ) ) {
			display = false;
			for ( const QString& str : line.split(QLatin1Char(' ') ) ) {
				bool ok;
				int id = str.toInt(&ok);
				if ( ok )
					Q_EMIT gameOfferRemoved(id);
			}
		} else if ( line.startsWith( QLatin1String("<s>") ) && seekExp.indexIn(line) > -1 ) {
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
			Q_EMIT gameOfferReceived ( offer );
		} else if ( line.startsWith( QLatin1String("<pf>") ) && challengeExp.indexIn ( line ) > -1 ) {
			display = false;
			FicsChallenge challenge;
			challenge.gameId = challengeExp.cap ( 1 ).toInt();
			challenge.player.first = challengeExp.cap ( 2 );
			int ratingPos = ( challengeExp.cap(2) == challengeExp.cap(3) ) ? 4 : 6;
			challenge.player.second = challengeExp.cap ( ratingPos ).toInt();
			Q_EMIT challengeReceived ( challenge );
		} else if ( line.startsWith( QLatin1String("<pr>") ) ) {
			display = false;
			for ( const QString& str : line.split( QLatin1Char(' ') ) ) {
				bool ok;
				int id = str.toInt(&ok);
				if ( ok )
					Q_EMIT challengeRemoved(id);
			}
		} else if ( gameStartedExp.indexIn ( line ) > -1 ) {
			qCDebug(LOG_KNIGHTS) << "Game Started" << line;
			type = ChatWidget::StatusMessage;
			QString player1 = gameStartedExp.cap ( 1 );
			QString player2 = gameStartedExp.cap ( 3 );
			Color color = NoColor;
			if ( player1 == otherPlayerName ) {
				color = Black;
				setPlayerName ( player2 );
			} else {
				color = White;
				setPlayerName ( player1 );
			}
			if ( byColor(color) != this ) {
				qCDebug(LOG_KNIGHTS) << "Switching protocols";
				// The color is different than was assigned at first
				// We have to switch the protocols
				Protocol* t = white();
				setWhiteProtocol(black());
				setBlackProtocol(t);
			}
			Protocol* opp = Protocol::byColor ( oppositeColor ( color ) );
			if ( opp->isLocal() )
				opp->setPlayerName ( otherPlayerName );

			m_stage = PlayStage;
			initComplete();
		}
		break;
	case PlayStage:
		if ( moveRegExp.indexIn ( line ) > -1 ) {
			display = false;
			qCDebug(LOG_KNIGHTS) << moveRegExp.cap(1) << colorName(color());
			bool validMove = !( moveRegExp.cap ( 1 ) == QLatin1Char('W') && color() == White )
			                 && !( moveRegExp.cap ( 1 ) == QLatin1Char('B') && color() == Black );

			const int whiteTimeLimit = moveRegExp.cap ( 3 ).toInt();
			const int blackTimeLimit = moveRegExp.cap ( 4 ).toInt();
			const QString  moveString = moveRegExp.cap ( 6 );

			qCDebug(LOG_KNIGHTS) << "Move:" << moveString;

			if ( moveString == QLatin1String("none") ) {
				TimeControl tc;
				tc.moves = 0;
				tc.baseTime = QTime().addSecs(whiteTimeLimit);
				tc.increment = moveRegExp.cap(2).toInt();
				Manager::self()->setTimeControl(NoColor, tc);
				break;
			}

			if ( validMove ) {
				Move m;
				if ( moveString == QLatin1String("o-o") ) {
					// Short (king's rook) castling
					m = Move::castling ( Move::KingSide, color() );
				} else if ( moveString == QLatin1String("o-o-o") ) {
					// Long (Queen's rock) castling
					m = Move::castling ( Move::QueenSide, color() );
				} else if ( moveStringExp.indexIn ( moveString ) > -1 ) {
					m.setFrom( Pos(moveStringExp.cap(1)) );
					m.setTo( Pos(moveStringExp.cap(2)) );
					if ( !moveStringExp.cap(3).isEmpty() ) {
						m.setFlag ( Move::Promote, true );
						QChar typeChar = moveRegExp.cap ( 3 ).mid ( 1, 1 ) [0];
						m.setPromotedType ( Piece::typeFromChar ( typeChar ) );
					}
				}
				qCDebug(LOG_KNIGHTS) << "Valid move" << m;
				Q_EMIT pieceMoved ( m );
			}
			Manager::self()->setCurrentTime ( White, QTime().addSecs ( whiteTimeLimit ) );
			Manager::self()->setCurrentTime ( Black, QTime().addSecs ( blackTimeLimit ) );

			if ( moveRegExp.cap(5).toInt() == 2 ) {
				// TODO: Notify the manager that time is starting now
			}
		} else if ( offerExp.indexIn(line) > -1 ) {
			Offer offer;
			offer.id = offerExp.cap(1).toInt();
			offer.player = color();
			QString type = offerExp.cap(3);
			if ( type == QLatin1String("abort") )
				offer.action = ActionAbort;
			else if ( type == QLatin1String("adjourn") )
				offer.action = ActionAdjourn;
			else if ( type == QLatin1String("draw") )
				offer.action = ActionDraw;
			else if ( type == QLatin1String("takeback") ) {
				offer.action = ActionUndo;
				offer.numberOfMoves = offerExp.cap(4).toInt();
			}
			m_offers.insert ( offer.id, offer );
			Manager::self()->sendOffer ( offer );
		} else if ( line.contains ( QLatin1String(" says:") ) ) {
			type = ChatWidget::ChatMessage;
			m_chat->addText ( line, type );
		} else if ( line.contains ( QLatin1String("lost contact or quit") ) )
			type = ChatWidget::AccountMessage;
		else if ( line.startsWith(QLatin1Char('{')) && line.contains(QLatin1Char('}'))) {
			if ( line.endsWith ( QLatin1String("1-0") ) ) {
				type = ChatWidget::AccountMessage;
				Q_EMIT gameOver ( White );
			} else if ( line.endsWith ( QLatin1String("1/2-1/2") ) || line.endsWith ( QLatin1Char('*') ) ) {
				// Knights has no way of reporting aborted or unfinished games
				// so we report aborted games as draws
				type = ChatWidget::AccountMessage;
				Q_EMIT gameOver ( NoColor );
			} else if ( line.endsWith ( QLatin1String("0-1") ) ) {
				type = ChatWidget::AccountMessage;
				Q_EMIT gameOver ( Black );
			}
		}
	}

	if ( display )
		writeToConsole ( line, type );
	return true;
}

Color FicsProtocol::parseColor ( QString str ) {
	if ( str.isEmpty() || str[0] == QLatin1Char('?') )
		return NoColor;
	if ( str[0] == QLatin1Char('W') )
		return White;
	if ( str[0] == QLatin1Char('B') )
		return Black;
	return NoColor;
}

void FicsProtocol::acceptSeek ( int id ) {
	write ( QLatin1String("play ") + QString::number(id) );
	m_seeking = false;
}

void FicsProtocol::acceptChallenge ( int id ) {
	write ( QLatin1String("accept ") + QString::number(id) );
	m_seeking = true;
}

void FicsProtocol::declineChallenge ( int id ) {
	write ( QLatin1String("decline ") + QString::number(id) );
}

void FicsProtocol::dialogRejected() {
	Q_EMIT error ( UserCancelled );
}

void FicsProtocol::setSeeking ( bool seek ) {
	m_seeking = seek;
	if ( seek ) {
		QByteArray seekText = "seek";

		TimeControl tc = Manager::self()->timeControl(color());
		seekText += ' ';
		seekText += QString::number ( 60 * tc.baseTime.hour() + tc.baseTime.minute() ).toLatin1();
		seekText += ' ';
		seekText += QString::number ( tc.increment ).toLatin1();

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
		qCDebug(LOG_KNIGHTS) << seekText;
		write(QLatin1String(seekText));
	} else
		write("unseek");
}

void FicsProtocol::resign() {
	write("resign");
}

void FicsProtocol::sendChat ( QString text ) {
	write ( QLatin1String("say ") + text );
}

void FicsProtocol::acceptOffer(const Offer& offer) {
	write ( QLatin1String("accept ") + QString::number(offer.id) );
}

void FicsProtocol::declineOffer(const Offer& offer) {
	write ( QLatin1String("decline ") + QString::number(offer.id) );
}

void FicsProtocol::makeOffer(const Offer& offer) {
	switch (offer.action) {
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
