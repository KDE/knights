/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_HISTORYWIDGET_H
#define KNIGHTS_HISTORYWIDGET_H

#include "core/move.h"

#include <QWidget>

namespace Ui {
class HistoryWidget;
}

namespace Knights {

class HistoryWidget : public QWidget {
	Q_OBJECT

public:
    explicit HistoryWidget(QWidget* parent = nullptr, Qt::WindowFlags f = {});
	~HistoryWidget() override;

private:
	Ui::HistoryWidget* ui;

private Q_SLOTS:
	void updateHistory();
};

}

#endif // KNIGHTS_HISTORYWIDGET_H
