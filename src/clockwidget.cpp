/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "clockwidget.h"
#include "knightsdebug.h"
#include "ui_clockwidget.h"

#include <QTime>

using namespace Knights;

ClockWidget::ClockWidget ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f ),
	ui(new Ui::ClockWidget),
	m_activePlayer(NoColor) {

	ui->setupUi ( this );
}

ClockWidget::~ClockWidget() {
	delete ui;
}

void ClockWidget::setDisplayedPlayer ( Color color ) {
	bool w = ( color == White );
	ui->verticalLayout->addWidget ( w ? ui->groupB : ui->groupW );
	ui->verticalLayout->addWidget ( w ? ui->groupW : ui->groupB );
}

void ClockWidget::setPlayerName ( Color color, const QString& name ) {
	switch ( color ) {
	case White:
		ui->groupW->setTitle ( name );
		break;
	case Black:
		ui->groupB->setTitle ( name );
		break;
	default:
		break;
	}
}

void ClockWidget::setCurrentTime ( Color color, const QTime& time ) {
	m_currentTime[color] = time;

	const int miliSeconds = time.hour() * 3600 * 1000 + time.minute() * 60 * 1000 + time.second() * 1000 + time.msec();
	const int units = miliSeconds / 100;
	QProgressBar* bar = ( color == White ) ? ui->progressW : ui->progressB;
	if ( units > bar->maximum() ) {
		bar->setMaximum ( units );
		updateTimeFormat();
	}
	bar->setValue ( units );
	bar->setFormat ( time.toString( m_timeFormat ) );

	Clock* clock = ( color == White ) ? ui->clockW : ui->clockB;
	clock->setTime ( time );
}

void ClockWidget::setTimeLimit ( Color color, const QTime& time ) {
	qCDebug(LOG_KNIGHTS) << color << time;
	m_timeLimit[color] = time;
	int seconds = time.hour() * 3600 + time.minute() * 60 + time.second();
	switch ( color ) {
	case White:
		ui->progressW->setMaximum ( seconds * 10 );
		break;
	case Black:
		ui->progressB->setMaximum ( seconds * 10 );
		break;
	default:
		break;
	}
	updateTimeFormat();
	setCurrentTime( color, time );
}

void ClockWidget::updateTimeFormat() {
	if ( m_timeLimit[White] > QTime(1,0) || m_timeLimit[Black] > QTime(1,0) )
		m_timeFormat = QStringLiteral("h:mm:ss");
	else
		m_timeFormat = QStringLiteral("mm:ss");
}
