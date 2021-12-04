/***************************************************************************
    File                 : DifficultyDialog.cpp
    Project              : Knights
    Description          : Dialogs for setting custom difficulty
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Alexander Semke (alexander.semke@web.de)

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
	setWindowTitle(i18nc("@title:window", "Difficulty Level"));

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
