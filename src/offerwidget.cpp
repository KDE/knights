/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009,2010,2011  Miha Čančula <miha@noughmad.eu>

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

#include "offerwidget.h"
#include "gamemanager.h"
#include "ui_popup.h"
#include <KStandardAction>

using namespace Knights;

#if defined WITH_KMW

OfferWidget::OfferWidget(const Knights::Offer& offer, QWidget* parent, Qt::WindowFlags f): KMessageWidget(offer.text, parent)
{
  Q_UNUSED(f)
  offerId = offer.id;
  if ( offer.action != ActionNone )
  {
    QAction* action;
    action = new QAction( KIcon(QLatin1String("dialog-ok")), i18n("Accept"), this );
    connect ( action, SIGNAL(triggered(bool)), SLOT(acceptClicked()) );
    addAction(action);
    action = new QAction( KIcon(QLatin1String("edit-delete")), i18n("Decline"), this );
    connect ( action, SIGNAL(triggered(bool)), SLOT(declineClicked()) );
    addAction ( action );
  }
  setCloseButtonVisible(true);
}

OfferWidget::~OfferWidget()
{

}

#else

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

#endif

int OfferWidget::id() const
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




