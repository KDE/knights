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

#include "knights.h"

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

static const char description[] =
    I18N_NOOP ( "Chess board based on KDE Development Platform 4" );

static const char version[] = "2.5.0";

int main ( int argc, char **argv )
{
    KAboutData about ( "knights", 0, ki18n ( "Knights" ), version, ki18n ( description ),
                       KAboutData::License_GPL, ki18n ( "(C) 2009-2011 Miha Čančula" ), KLocalizedString(), "miha@noughmad.eu" );
    about.addAuthor ( ki18n ( "Miha Čančula" ), KLocalizedString(), "miha@noughmad.eu", "http://noughmad.eu", "noughmad" );
    about.addCredit ( ki18n ( "Troy Corbin" ), ki18n ( "Original Knights for KDE3 and theme author" ), "troy@pedanticwebspaces.com" );
    about.addCredit ( ki18n ( "Dave Kaye" ), ki18n ( "Help with new theme features and rendering without KGameRenderer" ) );
    about.addCredit ( ki18n ( "Thomas Kamps" ), ki18n ( "Clock displaying the remaining time" ), QByteArray(), QByteArray(), "cpttom" );
    KCmdLineArgs::init ( argc, argv, &about );

    KCmdLineOptions options;
    options.add ( "+[URL]", ki18n ( "Document to open" ) );
    KCmdLineArgs::addCmdLineOptions ( options );
    KApplication app;
    KGlobal::locale()->insertCatalog ( QLatin1String ( "libkdegames" ) );
    
    // register types for connecting with Qt::QueuedConnection
    qRegisterMetaType<Knights::Color>("Color");

    // see if we are starting with session management
    if ( app.isSessionRestored() )
    {
        RESTORE ( Knights::MainWindow );
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
        if ( args->count() == 0 )
        {
            Knights::MainWindow *widget = new Knights::MainWindow;
            widget->show();
        }
        else
        {
            int i = 0;
            for ( ; i < args->count(); i++ )
            {
                Knights::MainWindow *widget = new Knights::MainWindow;
                widget->show();
            }
        }
        args->clear();
    }

    return app.exec();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
