/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_CLOCKWIDGET_H
#define KNIGHTS_CLOCKWIDGET_H

#include "core/piece.h"

#include <QWidget>

class QTime;

namespace Ui {
class ClockWidget;
}

namespace Knights {
class ClockWidget : public QWidget {
	Q_OBJECT
public:
    explicit ClockWidget ( QWidget* parent = nullptr, Qt::WindowFlags f = {} );
	~ClockWidget () override;

public Q_SLOTS:
	void setTimeLimit ( Color color, const QTime& time );
	void setDisplayedPlayer ( Color color );
	void setPlayerName ( Color color, const QString& name );
	void setCurrentTime ( Color color, const QTime& time );

private:
	Ui::ClockWidget* ui;
	Color m_activePlayer;
	QMap<Color, QTime> m_timeLimit;
	QMap<Color, QTime> m_currentTime;

	void updateTimeFormat();
	QString m_timeFormat;
};
}

#endif // KNIGHTS_CLOCKWIDGET_H
