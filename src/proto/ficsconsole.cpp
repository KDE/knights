/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha.cancula@gmail.com>

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

#include "proto/ficsconsole.h"
#include "ui_ficsconsole.h"

#include <KDebug>

using namespace Knights;

FicsConsole::FicsConsole ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
  ui = new Ui::FicsConsole;
  ui->setupUi(this);

  connect ( ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendButtonClicked()) );
  connect ( ui->acceptButton, SIGNAL(clicked(bool)), this, SLOT(acceptButtonClicked()) );
  connect ( ui->seekButton, SIGNAL(clicked(bool)), SLOT(seekButtonClicked()));
  connect ( ui->unseekButton, SIGNAL(clicked(bool)), SLOT(unseekButtonClicked()));

  QPalette p = ui->terminal->palette();
  p.setColor ( QPalette::Base, Qt::black );
  ui->terminal->setPalette ( p );
}

FicsConsole::~FicsConsole()
{

}

void FicsConsole::addText ( const QString text, QColor color )
{
  ui->terminal->moveCursor(QTextCursor::End);
  ui->terminal->setTextColor ( color );
  ui->terminal->textCursor().insertText( text + QLatin1Char('\n') );
  ui->terminal->moveCursor(QTextCursor::End);
  kDebug() << ui->terminal->textBackgroundColor() << ui->terminal->textColor();
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



