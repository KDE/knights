/***************************************************************************
    File                 : enginesettings.cpp
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

#include "enginesettings.h"
#include "ui_enginesettings.h"
#include "settings.h"

#include <KComboBox>

#include <QLabel>
#include <QStandardPaths>

using namespace Knights;

EngineConfiguration::EngineConfiguration(const QString& string) : iface(Invalid) {
	QStringList list = string.split(QLatin1Char(':'));
	if ( list.size() > 2 ) {
		name = list[0];
		commandLine = list[1];
		if ( list[2] == QLatin1String("xboard") )
			iface = XBoard;
		else if ( list[2] == QLatin1String("uci") )
			iface = Uci;
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

	ui->autoDetectButton->setIcon( QIcon::fromTheme(QStringLiteral("tools-wizard")) );
	ui->addButton->setIcon( QIcon::fromTheme(QStringLiteral("list-add")) );
	ui->removeButton->setIcon( QIcon::fromTheme(QStringLiteral("list-remove")) );

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
	connect(ui->autoDetectButton, &QPushButton::clicked, this, &EngineSettings::autoDetectEngines);
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

void EngineSettings::autoDetectEngines() {
	//descriptions for couple of known/popular open-source chess engines:
	//provide the name/Description of the engine, the name of the executable and the protocol,
	//keep the order within the three string lists in sync

	QStringList names;
	names << QStringLiteral("GNU Chess");
	names << QStringLiteral("Crafty");
	names << QStringLiteral("Stockfish");
	names << QStringLiteral("Stockfish v1.4");
	names << QStringLiteral("Stockfish v1.6");
	names << QStringLiteral("Stockfish v1.7");
	names << QStringLiteral("Stockfish v1.8");
	names << QStringLiteral("Stockfish v1.9");
	names << QStringLiteral("Stockfish v2.0");
	names << QStringLiteral("Sjeng");
	names << QStringLiteral("Phalanx");
	names << QStringLiteral("Fruit");
	names << QStringLiteral("Fruit v2.1");
	names << QStringLiteral("TogaII");
	names << QStringLiteral("TogaII v.1.4.2");
	names << QStringLiteral("Glaurung");
	names << QStringLiteral("HoiChess");

	QStringList commands;
	commands << QStringLiteral("gnuchess");
	commands << QStringLiteral("crafty");
	commands << QStringLiteral("stockfish");
	commands << QStringLiteral("stockfish14");
	commands << QStringLiteral("stockfish16");
	commands << QStringLiteral("stockfish17");
	commands << QStringLiteral("stockfish18");
	commands << QStringLiteral("stockfish19");
	commands << QStringLiteral("stockfish20");
	commands << QStringLiteral("sjeng");
	commands << QStringLiteral("phalanx");
	commands << QStringLiteral("fruit");
	commands << QStringLiteral("fruit21");
	commands << QStringLiteral("toga2");
	commands << QStringLiteral("togaII-142"); //openSUSE naming convention
	commands << QStringLiteral("glaurung");
	commands << QStringLiteral("hoichess");

	QVector<EngineConfiguration::Interface> interfaces;
	interfaces << EngineConfiguration::XBoard; //gnuchess
	interfaces << EngineConfiguration::XBoard; //crafty
	interfaces << EngineConfiguration::Uci; //stockfish
	interfaces << EngineConfiguration::Uci; //stockfish14
	interfaces << EngineConfiguration::Uci; //stockfish16
	interfaces << EngineConfiguration::Uci; //stockfish17
	interfaces << EngineConfiguration::Uci; //stockfish18
	interfaces << EngineConfiguration::Uci; //stockfish19
	interfaces << EngineConfiguration::Uci; //stockfish20
	interfaces << EngineConfiguration::XBoard; //sjeng
	interfaces << EngineConfiguration::XBoard; //phalanx //TODO: check this
	interfaces << EngineConfiguration::Uci; //fruit
	interfaces << EngineConfiguration::Uci; //fruit21
	interfaces << EngineConfiguration::Uci; //toga2
	interfaces << EngineConfiguration::Uci; //togaII-142
	interfaces << EngineConfiguration::Uci; //glaurung
	interfaces << EngineConfiguration::XBoard; //hoichess

	//remove all available engines first
	ui->tableWidget->setRowCount(0);

	//check the presence of the known engines defined above and add the corresponding entries in the table widget
	for (int i = 0; i < commands.size(); ++i) {
		const QString& command = commands.at(i);
		const bool exists = !QStandardPaths::findExecutable(command).isEmpty();
		if (!exists)
			continue;

		const int row = ui->tableWidget->rowCount();
		ui->tableWidget->insertRow(row);

		//name
		QTableWidgetItem* item = new QTableWidgetItem(names.at(i));
		ui->tableWidget->setItem(row, NameColumn, item);
		item->setFlags(item->flags() & (~Qt::ItemIsEditable));

		//command
		item = new QTableWidgetItem(command);
		ui->tableWidget->setItem(row, CommandColumn, item);
		item->setFlags(item->flags() & (~Qt::ItemIsEditable));

		//interface
		const QString interface = (interfaces.at(i) == EngineConfiguration::XBoard) ? QStringLiteral("XBoard") : QStringLiteral("UCI");
		item = new QTableWidgetItem(interface);
		ui->tableWidget->setItem(row, ProtocolColumn, item);
		item->setFlags(item->flags() & (~Qt::ItemIsEditable));

		//icon
		QLabel* label = new QLabel(this);
		label->setPixmap( QIcon::fromTheme(QStringLiteral("dialog-ok")).pixmap(32, 32) );
		ui->tableWidget->setCellWidget(row, InstalledColumn, label);
	}
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
		QWidget* w = ui->tableWidget->cellWidget(i, ProtocolColumn);
		const KComboBox* box = dynamic_cast<const KComboBox*>(w);
		if (box)
			c.iface = (EngineConfiguration::Interface)box->currentIndex();
		else if (ui->tableWidget->item(i, ProtocolColumn)->text() == QLatin1String("XBoard"))
			c.iface = EngineConfiguration::XBoard;
		else
			c.iface = EngineConfiguration::Uci;

		const QString str = c.toString();
		if(!str.isEmpty())
			out << c.toString();
	}

	Settings::setEngineConfigurations(out);
}

#include "moc_enginesettings.cpp"
