/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "offerwidget.h"
#include "gamemanager.h"
#include "ui_popup.h"

#include <QAction>

using namespace Knights;

OfferWidget::OfferWidget(const Knights::Offer& offer, QWidget* parent, Qt::WindowFlags f): KMessageWidget(offer.text, parent) {
	Q_UNUSED(f)
	offerId = offer.id;
	if ( offer.action != ActionNone ) {
		QAction* action;
		action = new QAction( QIcon::fromTheme(QStringLiteral("dialog-ok")), i18n("Accept"), this );
		connect ( action, &QAction::triggered, this, &OfferWidget::acceptClicked );
		addAction(action);
		action = new QAction( QIcon::fromTheme(QStringLiteral("edit-delete")), i18n("Decline"), this );
		connect ( action, &QAction::triggered, this, &OfferWidget::declineClicked );
		addAction ( action );
	}
	setCloseButtonVisible(true);
}

OfferWidget::~OfferWidget() {
	delete ui;
}

int OfferWidget::id() const {
	return offerId;
}

void OfferWidget::acceptClicked() {
	Q_EMIT close(offerId, AcceptOffer);
	deleteLater();
}

void OfferWidget::declineClicked() {
	Q_EMIT close(offerId, DeclineOffer);
	deleteLater();
}

void OfferWidget::closeClicked() {
	Q_EMIT close(offerId, IgnoreOffer);
	deleteLater();
}
