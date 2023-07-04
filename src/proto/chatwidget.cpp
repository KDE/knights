/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "proto/chatwidget.h"
#include "ui_chatwidget.h"

#include <QScrollBar>

using namespace Knights;

class ScrollBarPin {
public:
	ScrollBarPin(QScrollBar *scrollBar) : m_bar(scrollBar) {
		if (m_bar)
			m_bar = m_bar->value() == m_bar->maximum()? m_bar : nullptr;
	}
	~ScrollBarPin() {
		if (m_bar)
			m_bar->setValue(m_bar->maximum());
	}
private:
	QPointer<QScrollBar> m_bar;
};

void Terminal::resizeEvent ( QResizeEvent * event ) {
	ScrollBarPin b(verticalScrollBar());
	QTextBrowser::resizeEvent(event);
}

ChatWidget::ChatWidget ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f ) {
	ui = new Ui::ChatWidget;
	ui->setupUi(this);

	connect ( ui->sendButton, &QPushButton::clicked, this, &ChatWidget::sendButtonClicked );
	ui->sendButton->setIcon( QIcon::fromTheme(QStringLiteral("mail-send")) );

	m_terminal = new Terminal;
	ui->consoleLayout->addWidget(m_terminal);

	setConsoleMode ( false );
}

ChatWidget::~ChatWidget() {
	delete ui;
}

void ChatWidget::addText ( const QString& text, ChatWidget::MessageType type ) {
	ScrollBarPin b(m_terminal->verticalScrollBar());
	QTextCursor cursor = m_terminal->textCursor();
	QTextCharFormat format = cursor.charFormat();
	cursor.movePosition(QTextCursor::End);
	if ( type == ChatMessage && text.contains(QLatin1String(" says: ")) ) {
		format.setForeground( QBrush( messageColor ( StatusMessage ) ) );
		cursor.setCharFormat( format );
		cursor.insertText ( text.left ( text.indexOf(QLatin1String(" says: ")) ) );
		format.setForeground( QBrush( messageColor ( GeneralMessage ) ) );
		cursor.setCharFormat( format );
		cursor.insertText ( i18n(" says: ") );
		format.setForeground( QBrush( messageColor ( ChatMessage ) ) );
		cursor.setCharFormat( format );
		cursor.insertText ( text.mid ( text.indexOf(QLatin1String(" says: ")) + 7 ) );
	} else if (type == GreetMessage) {
		format.setFontItalic(true);
		format.setForeground( QBrush( messageColor ( type ) ) );
		cursor.setCharFormat( format );
		cursor.insertText( text );
	} else {
		format.setForeground( QBrush( messageColor ( type ) ) );
		cursor.setCharFormat( format );
		cursor.insertText( text );
	}
	cursor.insertText(QStringLiteral("\n"));
}

void ChatWidget::addText ( const QByteArray& text, ChatWidget::MessageType type ) {
	addText( QLatin1String(text), type );
}

void ChatWidget::addText(const Message& message) {
	addText ( message.first, message.second );
}

void ChatWidget::setPasswordMode ( bool pwMode ) {
    ui->line->setEchoMode(pwMode ? QLineEdit::Password : QLineEdit::Normal);
}

void ChatWidget::sendButtonClicked() {
	if ( m_consoleMode )
		addText ( ui->line->text(), GeneralMessage );
	else
		addText ( i18n("You: ") + ui->line->text(), ChatMessage );
	Q_EMIT sendText ( ui->line->text() );
	ui->line->clear();
}

void ChatWidget::addExtraButton ( const QString& text, const QString& title, const QString& icon ) {
	QPushButton* button = new QPushButton ( this );
	if ( !title.isEmpty() )
		button->setText ( title );
	else
		button->setText ( text );
	if ( !icon.isEmpty() )
		button->setIcon ( QIcon::fromTheme(icon) );
	ui->extraButtonsLayout->addWidget ( button );
	m_extraButtons.insert ( button, text );
	connect ( button, &QPushButton::clicked, this, &ChatWidget::buttonClicked );
}

void ChatWidget::buttonClicked() {
	QObject* s = sender();
	if ( m_extraButtons.contains ( s ) )
		Q_EMIT sendText ( m_extraButtons[s] );
}

QColor ChatWidget::messageColor ( ChatWidget::MessageType type ) const {
	if ( m_colors.contains ( type ) )
		return m_colors[type];
	return m_consoleMode ? Qt::white : Qt::black;
}

void ChatWidget::setMessageColor ( ChatWidget::MessageType type, const QColor& color ) {
	m_colors[type] = color;
}

void ChatWidget::setConsoleMode ( bool console ) {
	m_consoleMode = console;
	m_colors.clear();
	m_colors[AccountMessage] = Qt::magenta;
	m_colors[ErrorMessage] = Qt::red;
	m_colors[GreetMessage] = Qt::gray;
	m_colors[ChatMessage] = Qt::blue;
	m_colors[ChallengeMessage] = Qt::cyan;

	if ( console ) {
		QPalette p = m_terminal->palette();
		p.setColor ( QPalette::Base, Qt::black );
		m_terminal->setPalette ( p );

		m_colors[StatusMessage] = Qt::green;
		m_colors[SeekMessage] = Qt::yellow;
		m_colors[GeneralMessage] = Qt::white;
	} else {
		m_terminal->setPalette( palette() );

		m_colors[StatusMessage] = Qt::darkGreen;
		m_colors[GeneralMessage] = Qt::black;
		m_colors[SeekMessage] = Qt::darkYellow;
	}
}

bool ChatWidget::consoleMode() const {
	return m_consoleMode;
}

#include "moc_chatwidget.cpp"
