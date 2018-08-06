/***************************************************************************
    File                 : enginesettings.cpp
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

#include "enginesettings.h"
#include "ui_enginesettings.h"
#include "settings.h"

#include <KComboBox>

#include <QLabel>
#include <QStandardPaths>

using namespace Knights;

EngineConfiguration::EngineConfiguration(const QString& string) {
	QStringList list = string.split(QLatin1Char(':'));
	if ( list.size() > 2 ) {
		name = list[0];
		commandLine = list[1];
		if ( list[2] == QLatin1String("xboard") )
			iface = XBoard;
		else if ( list[2] == QLatin1String("uci") )
			iface = Uci;
		else
			iface = Invalid;
	}
}

const QString EngineConfiguration::toString() const {
	if ( iface == Invalid || commandLine.isEmpty() || name.isEmpty() )
		return QString();

	QLatin1Char sep(':');
	QString str = name + sep + commandLine + sep + ( iface == XBoard ? QLatin1String("xboard") : QLatin1String("uci") );
	return str;
}

EngineSettings::EngineSettings(QWidget* parent, Qt::WindowFlags f) : QWidget(parent, f), ui(new Ui::EngineSettings) {
	ui->setupUi(this);
	ui->addButton->setIcon ( QIcon::fromTheme(QLatin1String("list-add")) );
	ui->removeButton->setIcon ( QIcon::fromTheme(QLatin1String("list-remove")) );

	//add saved engines
	int row = 0;
	for ( const QString& s : Settings::engineConfigurations() ) {
		addClicked();
		EngineConfiguration c = EngineConfiguration ( s );
		ui->tableWidget->setItem ( row, NameColumn, new QTableWidgetItem ( c.name ) );
		ui->tableWidget->setItem ( row, CommandColumn, new QTableWidgetItem ( c.commandLine ) );
		qobject_cast<KComboBox*> ( ui->tableWidget->cellWidget ( row, ProtocolColumn ) )->setCurrentIndex ( (int)c.iface );
		checkInstalled(row, c.commandLine);
		++row;
	}

	ui->tableWidget->resizeColumnsToContents();

	//connects
	connect(ui->addButton, &QPushButton::clicked, this, &EngineSettings::addClicked);
	connect(ui->removeButton, &QPushButton::clicked, this, &EngineSettings::removeClicked);
	connect(ui->tableWidget, &QTableWidget::itemChanged, this, &EngineSettings::tableItemChanged);
}

EngineSettings::~EngineSettings() {
	delete ui;
}

void EngineSettings::checkInstalled(int row, const QString& name) {
	const bool exists = !QStandardPaths::findExecutable(name).isEmpty();
	const char* iconName = exists ? "dialog-ok" : "dialog-error";
	QLabel* label = new QLabel(this);
	label->setPixmap ( QIcon::fromTheme(QLatin1String(iconName)).pixmap(32, 32) );
	ui->tableWidget->setCellWidget(row, InstalledColumn, label );
}

void EngineSettings::tableItemChanged(QTableWidgetItem* item) {
	//if the name of the executable was changed, check whether it's available
	if (item->column() == CommandColumn)
		checkInstalled(item->row(), item->text());
}

void EngineSettings::addClicked() {
	int n = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow ( n );
	KComboBox* box = new KComboBox ( this );
	box->insertItems ( 0, QStringList() << i18nc("Protocol name", "XBoard") << i18nc("Protocol name", "UCI") );
	ui->tableWidget->setCellWidget ( n, ProtocolColumn, box );
	ui->tableWidget->edit ( ui->tableWidget->model()->index ( n, NameColumn ) );
}

void EngineSettings::removeClicked() {
	if ( ui->tableWidget->selectionModel()->selectedRows().isEmpty() )
		return;

	int i = ui->tableWidget->selectionModel()->selectedRows().first().row();
	if (i != -1)
		ui->tableWidget->removeRow(i);
}

void EngineSettings::save() {
	QStringList out;
	for ( int i = 0; i < ui->tableWidget->rowCount(); ++i ) {
		EngineConfiguration c = EngineConfiguration ( QString() );

		//name
		QTableWidgetItem* item = ui->tableWidget->item (i, NameColumn);
		if (!item) continue;
		c.name = item->text();

		//command
		item = ui->tableWidget->item (i, CommandColumn);
		if (!item) continue;
		c.commandLine = item->text();

		//interface
		c.iface = (EngineConfiguration::Interface)qobject_cast<KComboBox*>(ui->tableWidget->cellWidget ( i, ProtocolColumn ))->currentIndex();

		const QString str = c.toString();
		if(!str.isEmpty())
			out << c.toString();
	}

	Settings::setEngineConfigurations(out);
}
