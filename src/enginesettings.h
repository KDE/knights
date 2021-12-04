/***************************************************************************
    File                 : enginesettings.h
    Project              : Knights
    Description          : Engine Settings
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2018 Alexander Semke (alexander.semke@web.de)
    SPDX-FileCopyrightText: 2009-2011 Miha Čančula (miha@noughmad.eu)

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

#ifndef KNIGHTS_ENGINESETTINGS_H
#define KNIGHTS_ENGINESETTINGS_H

#include <QWidget>

class QTableWidgetItem;

namespace Ui {
class EngineSettings;
}

namespace Knights {

class EngineConfiguration {
public:
	enum Interface {
		XBoard = 0,
		Uci = 1,
		Invalid = 0x10
	};

	explicit EngineConfiguration(const QString&);

	const QString toString() const;

	QString name;
	QString commandLine;
	Interface iface;
};

class EngineSettings : public QWidget {
	Q_OBJECT

	enum Column {
		NameColumn = 0,
		CommandColumn = 1,
		ProtocolColumn = 2,
		InstalledColumn = 3,
		ColumnCount = 4
	};

public:
    explicit EngineSettings(QWidget* parent = nullptr, Qt::WindowFlags f = {});
	~EngineSettings() override;

private:
	Ui::EngineSettings* ui;
	QList<EngineConfiguration> configurations;

public Q_SLOTS:
	void save();

private Q_SLOTS:
	void autoDetectEngines();
	void addClicked();
	void removeClicked();
	void checkInstalled(int row, const QString& name);
	void tableItemChanged(QTableWidgetItem*);
};

}

#endif // KNIGHTS_ENGINESETTINGS_H
