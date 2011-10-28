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

#include <QLabel>
#include <QPointer>
#include "engineconfigdialog.h"


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

EngineSettings::EngineSettings(QWidget* parent, Qt::WindowFlags f): QWidget()
{
  ui = new Ui::EngineSettings;
  ui->setupUi ( this );
  
  ui->addButton->setIcon ( KIcon(QLatin1String("list-add")) );
  connect ( ui->addButton, SIGNAL(clicked(bool)), SLOT(addClicked()) );
  
  ui->modifyButton->setIcon ( KIcon(QLatin1String("document-edit")) );
  connect ( ui->modifyButton, SIGNAL(clicked(bool)), SLOT(modifyClicked()) );
  
  ui->removeButton->setIcon ( KIcon(QLatin1String("list-remove")) );
  connect ( ui->removeButton, SIGNAL(clicked(bool)), SLOT(removeClicked()) );
  
  foreach ( const QString s, Settings::engineConfigurations() )
  {
    addConfiguration ( EngineConfiguration ( s ) );
  }
  
  connect ( ui->tableWidget, SIGNAL(itemSelectionChanged()), SLOT(selectionChanged()));
}

EngineSettings::~EngineSettings()
{

}

EngineConfiguration EngineSettings::selectedConfiguration()
{
  return configurationAt ( selectedIndex() );
}

int EngineSettings::selectedIndex()
{
  QModelIndexList list = ui->tableWidget->selectionModel()->selectedRows();
  if ( list.isEmpty() )
  {
    return -1;
  }
  return list.first().row();
}

void EngineSettings::addConfiguration(const EngineConfiguration& config)
{
  kDebug() << config.name << config.commandLine << config.interfaceName();
  configurations << config;
  int i = configurations.size();
  ui->tableWidget->setRowCount ( i );
  editConfiguration ( i-1, config );
}

EngineConfiguration EngineSettings::configurationAt(int index)
{
  if ( index < 0 || index >= configurations.size() )
  {
    return EngineConfiguration ( QString() );
  }
  return configurations.at ( index );
}

void EngineSettings::editConfiguration(int i, const EngineConfiguration& config)
{
  ui->tableWidget->setItem ( i, 0, new QTableWidgetItem ( config.name ) );
  ui->tableWidget->setItem ( i, 1, new QTableWidgetItem ( config.commandLine ) );
  ui->tableWidget->setItem ( i, 2, new QTableWidgetItem ( config.interfaceName() ) );
  
  QString exe = config.commandLine.split ( QLatin1Char(' '), QString::KeepEmptyParts ).first();
  QLabel* label = new QLabel ();
  if ( !KStandardDirs::findExe ( exe ).isEmpty() )
  {
    label->setPixmap ( KIcon(QLatin1String("dialog-ok")).pixmap(32, 32) );
  }
  else
  {
    label->setPixmap ( KIcon(QLatin1String("dialog-error")).pixmap(32, 32) );
  }
  ui->tableWidget->setCellWidget ( i, 3, label );
}

void EngineSettings::addClicked()
{
  QPointer<KDialog> dlg = new KDialog(this);
  EngineConfigDialog* config = new EngineConfigDialog ( dlg );
  dlg->setMainWidget ( config );
  if ( dlg->exec() == KDialog::Accepted )
  {
    addConfiguration ( config->configuration() );
  }
  delete dlg;
}

void EngineSettings::modifyClicked()
{
  QPointer<KDialog> dlg = new KDialog(this);
  int i = selectedIndex();
  EngineConfigDialog* config = new EngineConfigDialog ( dlg );
  config->setConfiguration ( selectedConfiguration() );
  dlg->setMainWidget ( config );
  if ( dlg->exec() == KDialog::Accepted )
  {
    editConfiguration ( i, config->configuration() );
  }
  delete dlg;
}

void EngineSettings::removeClicked()
{
  int i = selectedIndex();
  if ( i == -1)
  {
    return;
  }
  
  configurations.removeAt ( i );
  ui->tableWidget->removeRow ( i );
}

void EngineSettings::selectionChanged()
{
  bool selected = ( selectedIndex() > -1 );
  ui->modifyButton->setEnabled ( selected );
  ui->removeButton->setEnabled ( selected );
}

void EngineSettings::writeConfig()
{
  QStringList out;
  foreach ( const EngineConfiguration& c, configurations )
  {
    out << c.toString();
  }
  Settings::setEngineConfigurations ( out );
}

