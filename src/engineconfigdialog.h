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

#ifndef KNIGHTS_ENGINECONFIGDIALOG_H
#define KNIGHTS_ENGINECONFIGDIALOG_H

#include <QWidget>
#include "enginesettings.h"

namespace Ui {
  class EngineConfigDialog;
}

namespace Knights {

class EngineConfigDialog : public QWidget
{

public:
    explicit EngineConfigDialog(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~EngineConfigDialog();
    
    EngineConfiguration configuration();
    void setConfiguration ( const EngineConfiguration& config );
    
private:
    Ui::EngineConfigDialog* ui;
};



}

#endif // KNIGHTS_ENGINECONFIGDIALOG_H