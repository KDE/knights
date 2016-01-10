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

#include "historywidget.h"
#include "ui_historywidget.h"
#include "core/move.h"
#include "gamemanager.h"
#include "knightsdebug.h"

#include <QStringListModel>
#include <QScrollBar>

using namespace Knights;

HistoryWidget::HistoryWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
    ui = new Ui::HistoryWidget;
    ui->setupUi(this);

    connect( ui->notationComboBox, static_cast<void (QComboBox::*)(int)> (&QComboBox::currentIndexChanged),
              this, &HistoryWidget::updateHistory );
    connect( Manager::self(), &Manager::historyChanged, this, &HistoryWidget::updateHistory );

    qCDebug(LOG_KNIGHTS);
}

HistoryWidget::~HistoryWidget()
{
    delete ui;
}

void HistoryWidget::updateHistory()
{
    Move::Notation notation;
    switch ( ui->notationComboBox->currentIndex() )
    {
    case 0:
        notation = Move::Algebraic;
        break;
    case 1:
        notation = Move::LongAlgebraic;
        break;
    case 2:
        notation = Move::Coordinate;
        break;
    default:
        notation = Move::Algebraic;
        break;
    }

    bool bottom = ui->listWidget->verticalScrollBar()->value() == ui->listWidget->verticalScrollBar()->maximum();
	ui->listWidget->clear();
    int i = 0;
    foreach ( const Move& move, Manager::self()->moveHistory() )
    {
        QString string = QString::number(i/2 + 1) + QLatin1String(". ");
        if ( i % 2 )
        {
            string += QLatin1String("... ");
        }
        ui->listWidget->addItem(string + move.stringForNotation(notation));
        ++i;
    }

    if (bottom)
    {
        ui->listWidget->scrollToBottom();
    }
}
