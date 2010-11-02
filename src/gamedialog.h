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

#ifndef KNIGHTS_GAMEDIALOG_H
#define KNIGHTS_GAMEDIALOG_H

#include "core/piece.h"
#include "proto/protocol.h"
#include "settings.h"

#include <QtGui/QWidget>

namespace Ui
{
    class GameDialog;
}

namespace Knights
{
    class GameDialog : public QWidget
    {
            Q_OBJECT
        public:
            explicit GameDialog ( QWidget* parent = 0, Qt::WindowFlags f = 0 );
            virtual ~GameDialog();

            Settings::EnumProtocol::type protocol() const;
            Color color() const;
            QTime playerTime() const;
            QTime opponentTime() const;
            QString program() const;
            QString server() const;
            bool timeLimit() const;
            int playerIncrement() const;
            int opponentIncrement() const;

            void writeConfig();

        private:
            Ui::GameDialog* ui;
            bool m_sameTime;
            bool m_hotseat;
            bool m_timeEnabled;

            bool m_forceSameTime;

            void updateTimeEdits();

        private slots:
            void timeEnabled ( bool enabled );
            void sameTimeChanged ( bool enabled );
            void hotseatModeToggled ( bool enabled );
            void ficsModeToggled ( bool enabled );
    };

}

#endif // KNIGHTS_GAMEDIALOG_H
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
