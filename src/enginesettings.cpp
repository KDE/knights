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

EngineSettings::EngineSettings(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f) {
	ui = new Ui::EngineSettings;
	ui->setupUi ( this );

	ui->addButton->setIcon ( QIcon::fromTheme(QLatin1String("list-add")) );
	connect ( ui->addButton, &QPushButton::clicked, this, &EngineSettings::addClicked );

	ui->removeButton->setIcon ( QIcon::fromTheme(QLatin1String("list-remove")) );
	connect ( ui->removeButton, &QPushButton::clicked, this, &EngineSettings::removeClicked );

	int row = 0;
	foreach ( const QString& s, Settings::engineConfigurations() ) {
		addClicked();
		EngineConfiguration c = EngineConfiguration ( s );
		ui->tableWidget->setItem ( row, NameColumn, new QTableWidgetItem ( c.name ) );
		ui->tableWidget->setItem ( row, CommandColumn, new QTableWidgetItem ( c.commandLine ) );
		qobject_cast<KComboBox*> ( ui->tableWidget->cellWidget ( row, ProtocolColumn ) )->setCurrentIndex ( (int)c.iface );
		++row;
	}

	checkInstalled();
	connect ( ui->tableWidget, &QTableWidget::itemChanged, this, &EngineSettings::checkInstalled );
}

EngineSettings::~EngineSettings() {
	delete ui;
}

void EngineSettings::checkInstalled() {
	int n = ui->tableWidget->rowCount();
	for ( int i = 0; i < n; ++i ) {
		QTableWidgetItem* item = ui->tableWidget->item ( i, CommandColumn );
		QLatin1Char s ( ' ' );
		bool ok = item && !item->text().isEmpty() && !QStandardPaths::findExecutable ( item->text().split ( s ).first() ).isEmpty();
		const char* iconName = ok ? "dialog-ok" : "dialog-error";
		QLabel* label = new QLabel ( this );
		label->setPixmap ( QIcon::fromTheme(QLatin1String(iconName)).pixmap(32, 32) );
		ui->tableWidget->setCellWidget ( i, InstalledColumn, label );
	}
}

void EngineSettings::addClicked() {
	int n = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow ( n );
	KComboBox* box = new KComboBox ( this );
	box->insertItems ( 0, QStringList() << i18nc("Protocol name", "XBoard") << i18nc("Protocol name", "UCI") );
	ui->tableWidget->setCellWidget ( n, ProtocolColumn, box );
	checkInstalled();
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
