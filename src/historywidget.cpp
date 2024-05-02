/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "historywidget.h"
#include "ui_historywidget.h"
#include "gamemanager.h"
#include "knightsdebug.h"

#include <QScrollBar>

#include <math.h>

using namespace Knights;

HistoryWidget::HistoryWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f) {
	ui = new Ui::HistoryWidget;
	ui->setupUi(this);

	ui->twMoves->setSelectionBehavior(QAbstractItemView::SelectItems);
	ui->twMoves->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->twMoves->horizontalHeaderItem(0)->setText(i18nc("@title:column", "White"));
	ui->twMoves->horizontalHeaderItem(1)->setText(i18nc("@title:column", "Black"));

	connect( ui->notationComboBox, static_cast<void (QComboBox::*)(int)> (&QComboBox::currentIndexChanged),
	         this, &HistoryWidget::updateHistory );
	connect( Manager::self(), &Manager::historyChanged, this, &HistoryWidget::updateHistory );

	qCDebug(LOG_KNIGHTS);
}

HistoryWidget::~HistoryWidget() {
	delete ui;
}

void HistoryWidget::updateHistory() {
	Move::Notation notation;
	switch ( ui->notationComboBox->currentIndex() ) {
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

	bool bottom = ui->twMoves->verticalScrollBar()->value() == ui->twMoves->verticalScrollBar()->maximum();

	ui->twMoves->clearContents();

	const QStack<Move> moveHistory = Manager::self()->moveHistory();
	ui->twMoves->setRowCount(ceil(double(moveHistory.size())/2));
	for (int i=1; i<=moveHistory.size(); ++i) {
		const Move& move = moveHistory.at(i-1);
		QTableWidgetItem* item = new QTableWidgetItem(move.stringForNotation(notation));
		const int row = ceil(double(i)/2)-1;
		const int column = i%2 ? 0 : 1;
		ui->twMoves->setItem(row, column, item);
	}

	if (bottom)
		ui->twMoves->scrollToBottom();
}

#include "moc_historywidget.cpp"
