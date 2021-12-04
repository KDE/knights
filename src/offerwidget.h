/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_OFFERWIDGET_H
#define KNIGHTS_OFFERWIDGET_H

#include <KMessageWidget>

namespace Ui {
class Popup;
}

namespace Knights {

class Offer;

enum OfferAction {
	AcceptOffer,
	DeclineOffer,
	IgnoreOffer
};

class OfferWidget : public KMessageWidget {
	Q_OBJECT

public:
    explicit OfferWidget(const Offer& offer, QWidget* parent = nullptr, Qt::WindowFlags f = {});
	~OfferWidget() override;

	int id() const;

private:
	Ui::Popup* ui;
	int offerId;

private Q_SLOTS:
	void acceptClicked();
	void declineClicked();
	void closeClicked();

Q_SIGNALS:
	void close(int id, OfferAction action);
};

}

#endif // KNIGHTS_OFFERWIDGET_H
