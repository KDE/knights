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

#ifndef KNIGHTS_CHATWIDGET_H
#define KNIGHTS_CHATWIDGET_H

#include <QtGui/QWidget>
#include <QtCore/QMap>

#include <KTextBrowser>

namespace Ui
{
  class ChatWidget;
}

namespace Knights
{

    class Terminal: public KTextBrowser {
      protected:
        virtual void resizeEvent ( QResizeEvent * event );
    };

    class ChatWidget : public QWidget
    {
      Q_OBJECT
      Q_ENUMS(MessageType)
      
    public:
    enum MessageType
      {
	  AccountMessage,
	  SeekMessage,
	  ChallengeMessage,
	  ChatMessage,
	  GreetMessage,
	  StatusMessage,
	  GeneralMessage,
	  ErrorMessage,
	  MoveMessage
      };

      typedef QPair<QString,MessageType> Message;

      explicit ChatWidget ( QWidget* parent = 0, Qt::WindowFlags f = 0 );
            virtual ~ChatWidget();

	    void addExtraButton ( const QString& text, const QString& title = QString(), const QString& icon = QString() );

	    QColor messageColor ( MessageType type ) const;
	    void setMessageColor ( MessageType type, const QColor& color );

	    void setConsoleMode ( bool console );
	    bool consoleMode() const;

    public slots:
	    void addText ( const QString& text, MessageType type );
	    void addText ( const QByteArray& text, MessageType type );
	    void addText ( const Message& message );
            void setPasswordMode ( bool pwMode );

    private slots:
            void sendButtonClicked ( );
            void buttonClicked ( );
    signals:
	    void sendText ( const QString& text );

    private:
      Ui::ChatWidget* ui;
      Terminal * m_terminal;
      QMap<QObject*, QString> m_extraButtons;
      QMap<MessageType, QColor> m_colors;
      bool m_consoleMode;
    };

}

#endif // KNIGHTS_CHATWIDGET_H
