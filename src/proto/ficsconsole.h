/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef KNIGHTS_FICSCONSOLE_H
#define KNIGHTS_FICSCONSOLE_H

#include <QtGui/QWidget>

namespace Ui
{
  class FicsConsole;
}

namespace Knights
{

    class FicsConsole : public QWidget
    {
      Q_OBJECT

      enum MessageType
      {
	  SeekMessage,
	  
      };

        public:
            explicit FicsConsole ( QWidget* parent = 0, Qt::WindowFlags f = 0 );
            virtual ~FicsConsole();

    public slots:
	    void addText ( const QString text, QColor color );
	    void addText ( const char* text, QColor color );
            void setPasswordMode ( bool pwMode );

    private slots:
            void sendButtonClicked ( );
            void acceptButtonClicked ( );
            void seekButtonClicked ( );
            void unseekButtonClicked ( );

    signals:
	    void sendText ( const QString& text );

    private:
      Ui::FicsConsole* ui;

    };

}

#endif // KNIGHTS_FICSCONSOLE_H
