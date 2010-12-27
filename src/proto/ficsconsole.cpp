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


#include "ficsconsole.h"
#include "ui_ficsconsole.h"

using namespace Knights;

FicsConsole::FicsConsole ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
  ui = new Ui::FicsConsole;
  ui->setupUi(this);

  connect ( ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendButtonClicked()) );
  connect ( ui->acceptButton, SIGNAL(clicked(bool)), this, SLOT(acceptButtonClicked()) );
  connect ( ui->seekButton, SIGNAL(clicked(bool)), SLOT(seekButtonClicked()));
  connect ( ui->unseekButton, SIGNAL(clicked(bool)), SLOT(unseekButtonClicked()));
}

FicsConsole::~FicsConsole()
{

}

void FicsConsole::addText ( const QString text, QColor color )
{
  QTextCharFormat format;
  format.setForeground( QBrush(color) );
  ui->terminal->moveCursor(QTextCursor::End);
  ui->terminal->textCursor().insertText( text + QLatin1Char('\n'), format );
  ui->terminal->moveCursor(QTextCursor::End);
}

void FicsConsole::addText ( const char* text, QColor color )
{
    addText( QLatin1String(text), color );
}

void FicsConsole::setPasswordMode ( bool pwMode )
{
    ui->line->setPasswordMode ( pwMode );
}

void FicsConsole::sendButtonClicked()
{
    emit sendText ( ui->line->text() );
}

void FicsConsole::acceptButtonClicked()
{
    emit sendText ( QLatin1String("accept") );
}

void FicsConsole::seekButtonClicked()
{
    emit sendText ( QLatin1String("seek") );
}

void FicsConsole::unseekButtonClicked()
{
    emit sendText ( QLatin1String("unseek") );
}



