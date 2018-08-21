/***************************************************************************
    File                 : xboardprotocol.h
    Project              : Knights
    Description          : Wrapper for the XBoard protocol
    --------------------------------------------------------------------
    Copyright            : (C) 2016 by Alexander Semke (alexander.semke@web.de)
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

#ifndef XBOARDPROTOCOL_H
#define XBOARDPROTOCOL_H

#include "proto/computerprotocol.h"

namespace Knights {

class XBoardProtocol : public ComputerProtocol {
	Q_OBJECT

public:
	explicit XBoardProtocol(QObject* parent = nullptr);
	~XBoardProtocol();

	virtual void move(const Move&);
	virtual Features supportedFeatures();

	virtual QList<ToolWidgetData> toolWidgets();

	virtual bool parseLine(const QString& line);
	virtual bool parseStub(const QString& line);

private:
	QString lastMoveString;
	bool m_resumePending;
	int m_moves;
	int m_increment;
	int m_baseTime;
	bool m_timeLimit;

public Q_SLOTS:
	virtual void init ();
	virtual void startGame();
	virtual void setWinner(Color);

	virtual void makeOffer(const Offer&);
	virtual void acceptOffer(const Offer&);
	virtual void declineOffer(const Offer&);

	virtual void setDifficulty(int depth, int memory);
};
}

#endif //XBOARDPROTOCOL_H
