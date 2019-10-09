/***************************************************************************
    File                 : knightsview.h
    Project              : Knights
    Description          : Main view of Knights
    --------------------------------------------------------------------
    Copyright            : (C) 2016-2017 by Alexander Semke (alexander.semke@web.de)
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
#ifndef KNIGHTSVIEW_H
#define KNIGHTSVIEW_H

#include "core/piece.h"
#include <QWidget>

class KgThemeProvider;

namespace Ui {
class KnightsView;
}

namespace Knights {

class Board;
class Offer;
class OfferWidget;

class KnightsView : public QWidget {
	Q_OBJECT
public:
	explicit KnightsView(QWidget*);
	~KnightsView() override;

	void setupBoard(KgThemeProvider*);
	void drawBoard(KgThemeProvider*);

private:
	Ui::KnightsView* ui;
	Board* m_board;
	bool m_showAllOffers;
	QList<OfferWidget*> m_offerWidgets;

protected:
	void resizeEvent(QResizeEvent*) override;

public Q_SLOTS:
	void settingsChanged();

private Q_SLOTS:
	void resizeScene();
	void showAllOffersToggled();

	void showPopup(const Offer&);
	void popupHidden(int id);
	void updateOffers();

Q_SIGNALS:
	void signalChangeStatusbar(const QString&);

	void displayedPlayerChanged(Color);

	void popupAccepted();
	void popupRejected();
};
}

#endif // KNIGHTSVIEW_H
