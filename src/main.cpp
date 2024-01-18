/***************************************************************************
    File                 : main.cpp
    Project              : Knights
    Description          : main function
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016-2017 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2009-2011 Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/


#include "knights.h"
#include "knights_version.h"

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>

static QString version = QStringLiteral ("2.6.0");

int main ( int argc, char **argv ) {
	QApplication app(argc, argv);

	KLocalizedString::setApplicationDomain(QByteArrayLiteral("knights"));
	KCrash::initialize();

	KAboutData about ( QStringLiteral("knights"), i18n ( "Knights" ),
	                   QStringLiteral(KNIGHTS_VERSION_STRING),
                       i18n( "KDE Chess Board" ),
	                   KAboutLicense::GPL, i18n ( "(C) 2016-2017 Alexander Semke, 2009-2011 Miha Čančula" ),
			   QString(),
			   QStringLiteral("https://apps.kde.org/knights"));
	about.addAuthor(i18n("Alexander Semke"), QString(), QStringLiteral("alexander.semke@web.de"));
	about.addAuthor ( i18n ( "Miha Čančula" ), QString(), QStringLiteral ("miha@noughmad.eu"),
	                  QStringLiteral ("https://noughmad.eu/"), QStringLiteral ("noughmad") );
	about.addCredit ( i18n ( "Troy Corbin" ), i18n ( "Original Knights for KDE3 and theme author" ),
	                  QStringLiteral ("troy@pedanticwebspaces.com") );
	about.addCredit ( i18n ( "Dave Kaye" ), i18n ( "Help with new theme features and rendering without KGameRenderer" ) );
	about.addCredit ( i18n ( "Thomas Kamps" ), i18n ( "Clock displaying the remaining time" ),
	                  QString(), QString(), QStringLiteral("cpttom") );

	app.setWindowIcon(QIcon::fromTheme(QStringLiteral("knights")));
	KAboutData::setApplicationData(about);

	QCommandLineParser parser;
	parser.addOption(QCommandLineOption( QStringLiteral ("+[URL]"), i18n ( "Document to open" ) ));
	about.setupCommandLine(&parser);
	parser.process(app);
	about.processCommandLine(&parser);

	// register types for connecting with Qt::QueuedConnection
	qRegisterMetaType<Knights::Color>("Color");

	// see if we are starting with session management
	if ( app.isSessionRestored() ) {
                kRestoreMainWindows<Knights::MainWindow>();
	} else {
		// no session.. just start up normally
		if ( parser.positionalArguments().isEmpty() ) {
			Knights::MainWindow *widget = new Knights::MainWindow;
			widget->show();
		} else {
			int i = 0;
			for ( ; i < parser.positionalArguments().count(); i++ ) {
				Knights::MainWindow *widget = new Knights::MainWindow;
				widget->show();
			}
		}
	}

	return app.exec();
}
