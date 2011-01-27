/*
    <one line to give the program's name and a brief idea of what it does.>
    Copyright (C) <year>  <name of author>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "offerwidget.h"
#include "gamemanager.h"
#include "ui_popup.h"

using namespace Knights;

OfferWidget::OfferWidget(const Offer& offer, QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  offerId = offer.id;
  ui = new Ui::Popup;
  ui->setupUi(this);

  ui->closeButton->setIcon ( KIcon(QLatin1String( "dialog-close" )) );
  connect ( ui->acceptButton, SIGNAL(clicked(bool)), SLOT(acceptClicked()) );

  if ( offer.action == ActionNone )
  {
    ui->acceptButton->hide();
    ui->declineButton->hide();
  }
  else
  {
    ui->acceptButton->setIcon ( KIcon(QLatin1String( "dialog-ok" )) );
    ui->declineButton->setIcon ( KIcon(QLatin1String( "edit-delete" )) );
    connect ( ui->declineButton, SIGNAL(clicked(bool)), SLOT(declineClicked()) );
    connect ( ui->closeButton, SIGNAL(clicked(bool)), SLOT(closeClicked()) );
  }
  ui->label->setText ( offer.text );
}

OfferWidget::~OfferWidget()
{
  delete ui;
}

int OfferWidget::id()
{
  return offerId;
}

void OfferWidget::acceptClicked()
{
  emit close(offerId, AcceptOffer);
  deleteLater();
}

void OfferWidget::declineClicked()
{
  emit close(offerId, DeclineOffer);
  deleteLater();
}

void OfferWidget::closeClicked()
{
  emit close(offerId, IgnoreOffer);
  deleteLater();
}




