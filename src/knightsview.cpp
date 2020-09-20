/***************************************************************************
    File                 : knightsview.cpp
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

#include "knightsview.h"
#include "proto/protocol.h"
#include "board.h"
#include "gamemanager.h"
#include "knightsdebug.h"
#include "offerwidget.h"

#include "ui_knightsview_base.h"
#include "ui_popup.h"

using namespace Knights;

KnightsView::KnightsView(QWidget* parent) : QWidget(parent),
	ui(new Ui::KnightsView),
	m_board(nullptr),
	m_showAllOffers(false) {

	ui->setupUi ( this );

	// By default, show only one offer (set showAll to true then toggle it)
	//TODO
	updateOffers();

	connect ( ui->showAllOffers, &QPushButton::clicked, this, &KnightsView::showAllOffersToggled );
	connect ( Manager::self(), &Manager::notification, this, &KnightsView::showPopup );
}

KnightsView::~KnightsView() {
	delete ui;
}

void KnightsView::drawBoard(KgThemeProvider* provider) {
	m_board = new Board(provider, this);
	ui->canvas->setScene(m_board);
	resizeScene();

	Colors playerColors;
	playerColors |= White;
	playerColors |= Black;
	m_board->setPlayerColors(playerColors);
}

void KnightsView::setupBoard(KgThemeProvider* provider) {
	if (m_board)
		delete m_board;

	m_board = new Board ( provider, this );
	ui->canvas->setScene ( m_board );
	resizeScene();

	connect ( Manager::self(), &Manager::pieceMoved, m_board, &Board::movePiece );
	connect ( Manager::self(), &Manager::activePlayerChanged, m_board, &Board::setCurrentColor );
	connect ( m_board, &Board::displayedPlayerChanged, this, &KnightsView::displayedPlayerChanged );
	connect ( m_board, &Board::pieceMoved, Manager::self(), &Manager::moveByBoard );

	Colors playerColors;
	if ( Protocol::white()->isLocal() )
		playerColors |= White;
	if ( Protocol::black()->isLocal() )
		playerColors |= Black;
	m_board->setPlayerColors(playerColors);
}

void KnightsView::settingsChanged() {
	Q_EMIT signalChangeStatusbar ( i18n ( "Settings changed" ) );
	if (m_board)
		m_board->updateTheme();
}

void KnightsView::resizeEvent(QResizeEvent* e) {
	Q_UNUSED(e)
	resizeScene();
}

void KnightsView::resizeScene() {
	if ( ui->canvas && m_board ) {
		m_board->setSceneRect ( ui->canvas->contentsRect() );
		m_board->updateGraphics();
		ui->canvas->setTransform ( QTransform() );
	}
}

void KnightsView::showPopup(const Offer& offer) {
	OfferWidget* widget = new OfferWidget(offer, ui->notificationWidget);
	connect ( widget, &OfferWidget::close, Manager::self(), &Manager::setOfferResult );
	connect ( widget, &OfferWidget::close, this, &KnightsView::popupHidden );
	m_offerWidgets << widget;
	updateOffers();
}

void KnightsView::showAllOffersToggled() {
	m_showAllOffers = !m_showAllOffers;
	updateOffers();
}

void KnightsView::popupHidden(int id) {
	qCDebug(LOG_KNIGHTS) << m_offerWidgets << id << m_showAllOffers;
	for (OfferWidget* widget : m_offerWidgets) {
		if ( widget->id() == id )
			m_offerWidgets.removeAll(widget);
	}
	updateOffers();
}

void KnightsView::updateOffers() {
	if ( m_offerWidgets.isEmpty() ) {
		ui->notificationWidget->hide();
		return;
	}
	QGridLayout* layout = qobject_cast<QGridLayout*>(ui->notificationWidget->layout());
	if ( !layout )
		return;
	ui->showAllOffers->setIcon ( QIcon::fromTheme(QLatin1String( m_showAllOffers ? "arrow-up-double" : "arrow-down-double" )) );
	ui->showAllOffers->setVisible ( m_offerWidgets.size() > 1 );
	for (OfferWidget* widget : m_offerWidgets) {
		layout->removeWidget ( widget );
		widget->hide();
	}
	if ( m_showAllOffers ) {
		for ( int i = 0; i < m_offerWidgets.size(); ++i ) {
			layout->addWidget ( m_offerWidgets[i], i+1, 0 );
			m_offerWidgets[i]->show();
		}
	} else {
		layout->addWidget ( m_offerWidgets.last(), 1, 0 );
		m_offerWidgets.last()->show();
	}
	ui->notificationWidget->show();
}
