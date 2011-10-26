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

#include "engineconfigdialog.h"

#include "ui_engineconfigdialog.h"

using namespace Knights;

EngineConfigDialog::EngineConfigDialog(QWidget* parent, Qt::WindowFlags f): QWidget()
{
  ui = new Ui::EngineConfigDialog;
  ui->setupUi ( this );
}

EngineConfigDialog::~EngineConfigDialog()
{
  delete ui;
}

EngineConfiguration EngineConfigDialog::configuration()
{
  EngineConfiguration c = EngineConfiguration ( QString() );
  c.name = ui->nameLineEdit->text();
  c.commandLine = ui->commandLineEdit->text();
  switch ( ui->protocolComboBox->currentIndex() )
  {
    case 0:
      c.iface = EngineConfiguration::XBoard;
      break;
      
    case 1:
      c.iface = EngineConfiguration::Uci;
      break;
      
    default:
      c.iface = EngineConfiguration::Invalid;
      break;
  }
  return c;
}

void EngineConfigDialog::setConfiguration(const EngineConfiguration& config)
{
  ui->nameLineEdit->setText ( config.name );
  ui->commandLineEdit->setText ( config.commandLine );
  int i = 0;
  switch ( config.iface )
  {
    case EngineConfiguration::XBoard:
      i = 0;
      break;
      
    case EngineConfiguration::Uci:
      i = 1;
      break;
      
    default:
      break;
  }
  ui->protocolComboBox->setCurrentIndex ( i );
}



