/***************************************************************************
    File                 : DifficultyDialog.h
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

#ifndef DIFFICULTYDIALOG_H
#define DIFFICULTYDIALOG_H

#include <QDialog>

namespace Ui {
class CustomDifficultyDialog;
}

class DifficultyDialog : public QDialog {
	Q_OBJECT

public:
    explicit DifficultyDialog (QWidget* parent = nullptr, Qt::WindowFlags f = {});
	~DifficultyDialog() override;

	int searchDepth() const;
	int memorySize() const;

private:
	Ui::CustomDifficultyDialog* ui;
};

#endif
