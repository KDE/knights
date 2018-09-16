/***************************************************************************
    File                 : main.cpp
    Project              : Knights
    Description          : main function
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2009-2011 by Miha Čančula (miha@noughmad.eu)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
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

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>

#include <QApplication>
#include <QCommandLineParser>

static const char description[] = I18N_NOOP ( "KDE Chess Board" );
static QString version = QStringLiteral ("2.6.0");

int main ( int argc, char **argv ) {
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication app(argc, argv);

	KLocalizedString::setApplicationDomain("knights");
	KCrash::initialize();

	KAboutData about ( QStringLiteral("knights"), i18n ( "Knights" ), version, i18n( description ),
	                   KAboutLicense::GPL, i18n ( "(C) 2016-2017 Alexander Semke, 2009-2011 Miha Čančula" ));
	about.addAuthor(i18n("Alexander Semke"), QString(), QStringLiteral("alexander.semke@web.de"));
	about.addAuthor ( i18n ( "Miha Čančula" ), QString(), QStringLiteral ("miha@noughmad.eu"),
	                  QStringLiteral ("http://noughmad.eu"), QStringLiteral ("noughmad") );
	about.addCredit ( i18n ( "Troy Corbin" ), i18n ( "Original Knights for KDE3 and theme author" ),
	                  QStringLiteral ("troy@pedanticwebspaces.com") );
	about.addCredit ( i18n ( "Dave Kaye" ), i18n ( "Help with new theme features and rendering without KGameRenderer" ) );
	about.addCredit ( i18n ( "Thomas Kamps" ), i18n ( "Clock displaying the remaining time" ),
	                  QString(), QString(), QStringLiteral("cpttom") );

	app.setWindowIcon(QIcon::fromTheme(QStringLiteral("knights")));
	KAboutData::setApplicationData(about);

	QCommandLineParser parser;
	parser.addVersionOption();
	parser.addHelpOption();
	parser.addOption(QCommandLineOption( QStringLiteral ("+[URL]"), i18n ( "Document to open" ) ));
	about.setupCommandLine(&parser);
	parser.process(app);
	about.processCommandLine(&parser);

	// register types for connecting with Qt::QueuedConnection
	qRegisterMetaType<Knights::Color>("Color");

	// see if we are starting with session management
	if ( app.isSessionRestored() ) {
		RESTORE ( Knights::MainWindow );
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
