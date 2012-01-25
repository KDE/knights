/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2011  Miha Čančula <miha@noughmad.eu>

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

#include "enginesettings.h"
#include "ui_enginesettings.h"
#include "settings.h"

#include <KStandardDirs>
#include <KComboBox>

#include <QLabel>

using namespace Knights;

EngineConfiguration::EngineConfiguration(const QString& string)
{
  QString s = string;
  QStringList list = string.split(QLatin1Char(':'));
  if ( list.size() > 2 )
  {
    name = list[0];
    commandLine = list[1];
    if ( list[2] == QLatin1String("xboard") )
    {
      iface = XBoard;
    }
    else if ( list[2] == QLatin1String("uci") )
    {
      iface = Uci;
    }
    else
    {
      iface = Invalid;
    }
  }
}

EngineConfiguration::~EngineConfiguration()
{

}

QString EngineConfiguration::toString() const
{
  if ( iface == Invalid || commandLine.isEmpty() || name.isEmpty() )
  {
    return QString();
  }
  
  QLatin1Char sep(':');
  QString str = name + sep + commandLine + sep + ( iface == XBoard ? QLatin1String("xboard") : QLatin1String("uci") );
  return str;
}

QString EngineConfiguration::interfaceName() const
{
  switch ( iface )
  {
    case Invalid:
      return i18n("Invalid interface");
      
    case XBoard:
      return i18nc("Protocol name", "XBoard");
      
    case Uci:
      return i18nc("Protocol name", "UCI");
  }
  return QString();
}

EngineSettings::EngineSettings(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  ui = new Ui::EngineSettings;
  ui->setupUi ( this );
  
  ui->addButton->setIcon ( KIcon(QLatin1String("list-add")) );
  connect ( ui->addButton, SIGNAL(clicked(bool)), SLOT(addClicked()) );
  
  ui->removeButton->setIcon ( KIcon(QLatin1String("list-remove")) );
  connect ( ui->removeButton, SIGNAL(clicked(bool)), SLOT(removeClicked()) );
  
  int row = 0;
  foreach ( const QString& s, Settings::engineConfigurations() )
  {
    addClicked();
    EngineConfiguration c = EngineConfiguration ( s );
    ui->tableWidget->setItem ( row, NameColumn, new QTableWidgetItem ( c.name ) );
    ui->tableWidget->setItem ( row, CommandColumn, new QTableWidgetItem ( c.commandLine ) );
    qobject_cast<KComboBox*> ( ui->tableWidget->cellWidget ( row, ProtocolColumn ) )->setCurrentIndex ( (int)c.iface );
    ++row;
  }
  
  checkInstalled();
  connect ( ui->tableWidget, SIGNAL(itemChanged(QTableWidgetItem*)), SLOT(checkInstalled()) );
}

EngineSettings::~EngineSettings()
{

}

void EngineSettings::checkInstalled()
{
  int n = ui->tableWidget->rowCount();
  for ( int i = 0; i < n; ++i )
  {
    QTableWidgetItem* item = ui->tableWidget->item ( i, CommandColumn );
    QLatin1Char s ( ' ' );
    bool ok = item && !item->text().isEmpty() && !KStandardDirs::findExe ( item->text().split ( s ).first() ).isEmpty();
    const char* iconName = ok ? "dialog-ok" : "dialog-error"; 
    QLabel* label = new QLabel ( this );
    label->setPixmap ( KIcon(QLatin1String(iconName)).pixmap(32, 32) );
    ui->tableWidget->setCellWidget ( i, InstalledColumn, label );
  }
}

void EngineSettings::addClicked()
{
  int n = ui->tableWidget->rowCount();
  ui->tableWidget->insertRow ( n );
  KComboBox* box = new KComboBox ( this );
  box->insertItems ( 0, QStringList() << i18nc("Protocol name", "XBoard") << i18nc("Protocol name", "UCI") );
  ui->tableWidget->setCellWidget ( n, ProtocolColumn, box );
  checkInstalled();
  ui->tableWidget->edit ( ui->tableWidget->model()->index ( n, NameColumn ) );
}

void EngineSettings::removeClicked()
{
  if ( ui->tableWidget->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }
  
  int i = ui->tableWidget->selectionModel()->selectedRows().first().row();
  if ( i == -1)
  {
    return;
  }
  ui->tableWidget->removeRow ( i );
}

void EngineSettings::writeConfig()
{
  QStringList out;
  for ( int i = 0; i < ui->tableWidget->rowCount(); ++i )
  {
    EngineConfiguration c = EngineConfiguration ( QString() );
    c.name = ui->tableWidget->item ( i, NameColumn )->text();
    c.commandLine = ui->tableWidget->item ( i, CommandColumn )->text();
    c.iface = (EngineConfiguration::Interface)qobject_cast<KComboBox*>(ui->tableWidget->cellWidget ( i, ProtocolColumn ))->currentIndex();    
    out << c.toString();
  }
  Settings::setEngineConfigurations ( out );
}

