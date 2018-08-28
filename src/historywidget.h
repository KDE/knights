/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>
    Copyright 2016 Alexander Semke <alexander.semke@web.de>

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
	explicit HistoryWidget(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
	~HistoryWidget() override;

private:
	Ui::HistoryWidget* ui;

private slots:
	void updateHistory();
};

}

#endif // KNIGHTS_HISTORYWIDGET_H
