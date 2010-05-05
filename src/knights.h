/*
	This file is part of Knights, a chess board for KDE SC 4.
	Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>

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

#ifndef KNIGHTS_H
#define KNIGHTS_H

#include <KXmlGuiWindow>

#include "ui_prefs_base.h"

class QPrinter;
class KToggleAction;
class KUrl;


namespace Knights
{
class Protocol;
  class KnightsView;
  
  /**
 * This class serves as the main window for Knights.  It handles the
 * menus, toolbars, and status bars.
 *
 * @short Main window class
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */
class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    /**
     * Default Constructor
     */
    MainWindow();

    /**
     * Default Destructor
     */
    virtual ~MainWindow();

private slots:
    void fileNew();
    void pauseGame(bool pause);
    void optionsPreferences();

private:
    void setupActions();

private:
    Ui::prefs_base ui_prefs_base ;
    KnightsView *m_view;
    QPointer<QDockWidget> m_clockDock;

    KToggleAction *m_toolbarAction;
    KToggleAction *m_statusbarAction;

    QPointer<Protocol> m_protocol;

};

}

#endif // _KNIGHTS_H_
