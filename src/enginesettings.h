/***************************************************************************
    File                 : enginesettings.h
    Project              : Knights
    Description          : Engine Settings
    --------------------------------------------------------------------
    Copyright            : (C) 2018 by Alexander Semke (alexander.semke@web.de)
    Copyright            : (C) 2009-2011 by Miha Čančula (miha@noughmad.eu)

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
	explicit EngineSettings(QWidget* parent = nullptr, Qt::WindowFlags f = nullptr);
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
