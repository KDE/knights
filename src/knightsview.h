/***************************************************************************
    File                 : knightsview.h
    Project              : Knights
    Description          : Main view of Knights
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
#ifndef KNIGHTSVIEW_H
#define KNIGHTSVIEW_H

#include "core/piece.h"
#include "offerwidget.h"
#include <QWidget>

class KgThemeProvider;

namespace Ui {
class KnightsView;
}

namespace Knights {

class Board;
struct Offer;

class KnightsView : public QWidget {
	Q_OBJECT
public:
	explicit KnightsView(QWidget*);
	virtual ~KnightsView();

	void setupBoard(KgThemeProvider*);
	void drawBoard(KgThemeProvider*);

private:
	Ui::KnightsView* ui;
	Board* m_board;
	bool m_showAllOffers;
	bool m_allOffers;
	QList<OfferWidget*> m_offerWidgets;

protected:
	virtual void resizeEvent(QResizeEvent*);

public slots:
	void settingsChanged();

private slots:
	void resizeScene();
	void showAllOffersToggled();

	void gameOver(Color);
	void showPopup(const Offer&);
	void popupHidden(int id);
	void updateOffers();

signals:
	void signalChangeStatusbar(const QString&);
	void gameNew();

	void activePlayerChanged(Color);
	void displayedPlayerChanged(Color);

	void popupAccepted();
	void popupRejected();
};
}

#endif // KNIGHTSVIEW_H
