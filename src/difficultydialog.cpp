/***************************************************************************
    File                 : DifficultyDialog.cpp
    Project              : Knights
    Description          : Dialogs for setting custom difficulty
    --------------------------------------------------------------------
    Copyright            : (C) 2016 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "difficultydialog.h"
#include "ui_customdifficultydialog.h"
#include "settings.h"

#include <QDialogButtonBox>

DifficultyDialog::DifficultyDialog(QWidget* parent, Qt::WindowFlags f) : QDialog(parent, f) {
	QFrame* mainFrame = new QFrame(this);
	ui = new Ui::CustomDifficultyDialog();
	ui->setupUi(mainFrame);
	ui->sbSearchDepth->setSuffix(ki18ncp("Search depth suffix", " move", " moves"));
	ui->sbSearchDepth->setValue(Settings::computerSearchDepth());
	ui->sbMemorySize->setValue(Settings::computerMemorySize());

	QDialogButtonBox *bBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);

	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(mainFrame);
	layout->addWidget(bBox);

	setLayout(layout);
	setWindowTitle(i18n("Difficulty Level"));

	connect(bBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
	connect(bBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
}

DifficultyDialog::~DifficultyDialog() {
	delete ui;
}

int DifficultyDialog::memorySize() const {
	return ui->sbMemorySize->value();
}

int DifficultyDialog::searchDepth() const {
	return ui->sbSearchDepth->value();
}
