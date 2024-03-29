/***************************************************************************
    File                 : xboardprotocol.h
    Project              : Knights
    Description          : Wrapper for the XBoard protocol
    --------------------------------------------------------------------
    SPDX-FileCopyrightText: 2016 Alexander Semke (alexander.semke@web.de)
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

#ifndef XBOARDPROTOCOL_H
#define XBOARDPROTOCOL_H

#include "proto/computerprotocol.h"

namespace Knights {

class XBoardProtocol : public ComputerProtocol {
	Q_OBJECT

public:
	explicit XBoardProtocol(QObject* parent = nullptr);
	~XBoardProtocol() override;

	void move(const Move&) override;
	Features supportedFeatures() override;

	QList<ToolWidgetData> toolWidgets() override;

	bool parseLine(const QString& line) override;
	bool parseStub(const QString& line) override;

private:
	QString lastMoveString;
	bool m_resumePending;
	int m_moves;
	int m_increment;
	int m_baseTime;
	bool m_timeLimit;

public Q_SLOTS:
	void init () override;
	void startGame() override;
	void setWinner(Color) override;

	void makeOffer(const Offer&) override;
	void acceptOffer(const Offer&) override;
	void declineOffer(const Offer&) override;

	void setDifficulty(int depth, int memory) override;
};
}

#endif //XBOARDPROTOCOL_H
