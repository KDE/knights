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

#include "proto/chatwidget.h"
#include "ui_chatwidget.h"

#include <KDebug>

using namespace Knights;

ChatWidget::ChatWidget ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
  ui = new Ui::ChatWidget;
  ui->setupUi(this);

  connect ( ui->sendButton, SIGNAL(clicked(bool)), this, SLOT(sendButtonClicked()) );
  ui->sendButton->setIcon( KIcon(QLatin1String("mail-send")) );

  setConsoleMode ( false );
}

ChatWidget::~ChatWidget()
{

}

void ChatWidget::addText ( const QString text, ChatWidget::MessageType type )
{
  ui->terminal->moveCursor(QTextCursor::End);
  ui->terminal->setTextColor ( messageColor ( type ) );
  ui->terminal->textCursor().insertText( text + QLatin1Char('\n') );
  ui->terminal->moveCursor(QTextCursor::End);
  kDebug() << ui->terminal->textBackgroundColor() << ui->terminal->textColor();
}

void ChatWidget::addText ( const char* text, ChatWidget::MessageType type )
{
    addText( QLatin1String(text), type );
}

void ChatWidget::setPasswordMode ( bool pwMode )
{
    ui->line->setPasswordMode ( pwMode );
}

void ChatWidget::sendButtonClicked()
{
    emit sendText ( ui->line->text() );
}

void ChatWidget::addExtraButton ( const QString& text, const QString& title, const QString& icon )
{
    KPushButton* button = new KPushButton ( this );
    if ( !title.isEmpty() )
    {
	button->setText ( title );
    }
    else
    {
	button->setText ( text );
    }
    if ( !icon.isEmpty() )
    {
	button->setIcon ( KIcon(icon) );
    }
    ui->extraButtonsLayout->addWidget ( button );
    m_extraButtons.insert ( button, text );
    connect ( button, SIGNAL(clicked(bool)), SLOT(buttonClicked()) );
}

void ChatWidget::buttonClicked()
{
    QObject* s = sender();
    if ( m_extraButtons.contains ( s ) )
    {
	emit sendText ( m_extraButtons[s] );
    }
}

QColor ChatWidget::messageColor ( ChatWidget::MessageType type )
{
  if ( m_colors.contains ( type ) )
  {
    return m_colors[type];
  }
  return m_consoleMode ? Qt::white : Qt::black;
}

void ChatWidget::setMessageColor ( ChatWidget::MessageType type, const QColor& color )
{
  m_colors[type] = color;
}

void ChatWidget::setConsoleMode ( bool console )
{
  m_consoleMode = console;
  m_colors.clear();
  m_colors[AccountMessage] = Qt::red;
  m_colors[StatusMessage] = Qt::green;
  m_colors[GreetMessage] = Qt::gray;
  m_colors[ChatMessage] = Qt::blue;
  m_colors[ChallengeMessage] = Qt::cyan;

  if ( console )
  {
    QPalette p = ui->terminal->palette();
    p.setColor ( QPalette::Base, Qt::black );
    ui->terminal->setPalette ( p );

    m_colors[SeekMessage] = Qt::yellow;
    m_colors[GeneralMessage] = Qt::white;
  }
  else
  {
    ui->terminal->setPalette( palette() );

    m_colors[GeneralMessage] = Qt::black;
    m_colors[SeekMessage] = Qt::darkYellow;
  }
}

bool ChatWidget::consoleMode()
{
  return m_consoleMode;
}






