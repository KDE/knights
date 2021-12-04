/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_CHATWIDGET_H
#define KNIGHTS_CHATWIDGET_H

#include <QWidget>
#include <QMap>
#include <QTextBrowser>

namespace Ui {
class ChatWidget;
}

namespace Knights {

class Terminal: public QTextBrowser {
protected:
	void resizeEvent ( QResizeEvent * event ) override;
};

class ChatWidget : public QWidget {
	Q_OBJECT
	Q_ENUMS(MessageType)

public:
	enum MessageType {
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

    explicit ChatWidget ( QWidget* parent = nullptr, Qt::WindowFlags f = {} );
	~ChatWidget() override;

	void addExtraButton ( const QString& text, const QString& title = QString(), const QString& icon = QString() );

	QColor messageColor ( MessageType type ) const;
	void setMessageColor ( MessageType type, const QColor& color );

	void setConsoleMode ( bool console );
	bool consoleMode() const;

public Q_SLOTS:
	void addText ( const QString& text, MessageType type );
	void addText ( const QByteArray& text, MessageType type );
	void addText ( const Message& message );
	void setPasswordMode ( bool pwMode );

private Q_SLOTS:
	void sendButtonClicked ( );
	void buttonClicked ( );
Q_SIGNALS:
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
