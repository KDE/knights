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

#include "knightsview.h"
#include "ui_popup.h"

#include "core/pos.h"
#include "proto/protocol.h"
#include "knights.h"
#include "settings.h"
#include "board.h"

#include <KMessageBox>
#include <KLocale>

#include <QtGui/QLabel>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QEvent>
#include "gamemanager.h"
#include "offerwidget.h"
#include "ui_knightsview_base.h"
#include <KActionCollection>
#include <KAction>
#include <KStandardGameAction>

using namespace Knights;

KnightsView::KnightsView ( QWidget *parent )
        : QWidget ( parent )
        , ui ( new Ui::KnightsView )
{
    ui->setupUi ( this );

    // By default, show only one offer (set showAll to true then toggle it)
    m_showAllOffers = false;
    updateOffers();
    
    connect ( ui->showAllOffers, SIGNAL(clicked(bool)), SLOT(showAllOffersToggled()) );
    connect ( Manager::self(), SIGNAL(notification(Offer)), SLOT(showPopup(Offer)) );
    connect ( Manager::self(), SIGNAL(winnerNotify(Color)), SLOT (gameOver(Color)), Qt::QueuedConnection );
    connect ( Manager::self(), SIGNAL(activePlayerChanged(Color)), SIGNAL(activePlayerChanged(Color)) );
    
    m_board = 0;
    settingsChanged();
}

KnightsView::~KnightsView()
{
    delete ui;
}

void KnightsView::setupBoard(KgThemeProvider* provider)
{
    m_board = new Board ( provider, this );
    ui->canvas->setScene ( m_board );
    resizeScene();
    kDebug() << Manager::self();
    connect ( Manager::self(), SIGNAL(pieceMoved(Move)), m_board, SLOT(movePiece(Move)) );
    connect ( Manager::self(), SIGNAL(activePlayerChanged(Color)), m_board, SLOT(setCurrentColor(Color)) );
    connect ( m_board, SIGNAL(displayedPlayerChanged(Color)), SIGNAL(displayedPlayerChanged(Color)) );
    connect ( m_board, SIGNAL(pieceMoved(Move)), Manager::self(), SLOT(moveByBoard(Move)) );

    Colors playerColors;
    if ( Protocol::white()->isLocal() )
    {
        playerColors |= White;
    }
    if ( Protocol::black()->isLocal() )
    {
        playerColors |= Black;
    }
    m_board->setPlayerColors(playerColors);
}

void KnightsView::clearBoard()
{
    kDebug();
    delete m_board;
    m_board = 0;
}

void KnightsView::gameOver ( Color winner )
{
    kDebug() << sender() << colorName ( winner );
    
    QPointer<KDialog> dlg = new KDialog ( this );
    dlg->setButtons ( KDialog::Yes | KDialog::User2 | KDialog::Cancel );
    
    KActionCollection* c = qobject_cast<KXmlGuiWindow*>( parentWidget() )->actionCollection();
    Q_ASSERT(c);
    
    QMap<KDialog::ButtonCode, QByteArray> buttonsMap;
    
    buttonsMap[KDialog::Yes] = KStandardGameAction::name ( KStandardGameAction::New );
    buttonsMap[KDialog::User2] = KStandardGameAction::name ( KStandardGameAction::Save );
    
    for ( QMap<KDialog::ButtonCode, QByteArray>::ConstIterator it = buttonsMap.constBegin(); it != buttonsMap.constEnd(); ++it )
    {
        QAction* a = c->action ( QLatin1String ( it.value() ) );
        Q_ASSERT(a);

        dlg->button ( it.key() )->setText ( a->text() );
        dlg->button ( it.key() )->setIcon ( KIcon ( a->icon() ) );
        dlg->button ( it.key() )->setToolTip ( a->toolTip() );
    }
        
    connect ( dlg->button ( KDialog::User2 ), SIGNAL(clicked(bool)), window(), SLOT(fileSave()) );
    
    QLabel* label = new QLabel ( this );
    dlg->setMainWidget ( label );
    dlg->setCaption ( i18n("Game over") );
    

    if ( winner == NoColor )
    {
        label->setText ( i18n ( "The game ended in a draw" ) );
    }
    else
    {
        QString winnerName = Protocol::byColor ( winner )->playerName();
        if ( winnerName == colorName(winner) )
        {
            if ( winner == White )
            {
                label->setText ( i18nc("White as in the player with white pieces", 
                                       "The game ended with a victory for <emphasis>White</emphasis>") );
            }
            else
            {                
                label->setText ( i18nc("Black as in the player with black pieces", 
                                       "The game ended with a victory for <emphasis>Black</emphasis>") );
            }
        }
        else
        {
            if ( winner == White )
            {
                label->setText ( i18nc("Player name, then <White as in the player with white pieces", 
                                       "The game ended with a victory for <emphasis>%1</emphasis>, playing White", winnerName) );
            }
            else
            {                
                label->setText ( i18nc("Player name, then Black as in the player with black pieces", 
                                       "The game ended with a victory for <emphasis>%1</emphasis>, playing Black", winnerName) );
            }
        }
    }
    
    if ( dlg->exec() == KDialog::Yes )
    {
        Manager::self()->reset();
        emit gameNew();
    }
    
    kDebug() << Protocol::white();
    kDebug() << Protocol::black();
    delete dlg;
}

void KnightsView::settingsChanged()
{
    emit signalChangeStatusbar ( i18n ( "Settings changed" ) );
    if ( m_board )
    {
        m_board->updateTheme();
    }
}

void KnightsView::resizeEvent ( QResizeEvent* e )
{
    Q_UNUSED ( e )
    resizeScene();
}

void KnightsView::resizeScene()
{
    if ( ui->canvas && m_board )
    {
        m_board->setSceneRect ( ui->canvas->contentsRect() );
        m_board->updateGraphics();
        ui->canvas->setTransform ( QTransform() );
    }
}

void KnightsView::centerView ( const QPointF& center )
{
    if ( ui->canvas )
    {
        ui->canvas->centerOn ( center );
    }
}

void KnightsView::showPopup(const Offer& offer)
{
    OfferWidget* widget = new OfferWidget(offer, ui->notificationWidget);
    connect ( widget, SIGNAL(close(int,OfferAction)), Manager::self(), SLOT(setOfferResult(int,OfferAction)) );
    connect ( widget, SIGNAL(close(int,OfferAction)), SLOT(popupHidden(int)));
    m_offerWidgets << widget;
    updateOffers();
}

void KnightsView::showAllOffersToggled()
{
    m_showAllOffers = !m_showAllOffers;
    updateOffers();
}

void KnightsView::popupHidden(int id)
{
    kDebug() << m_offerWidgets << id << m_showAllOffers;
    foreach ( OfferWidget* widget, m_offerWidgets )
    {
        if ( widget->id() == id )
        {
            m_offerWidgets.removeAll(widget);
        }
    }
    updateOffers();
}

void KnightsView::updateOffers()
{
    if ( m_offerWidgets.isEmpty() )
    {
        ui->notificationWidget->hide();
        return;
    }
    QGridLayout* layout = qobject_cast<QGridLayout*>(ui->notificationWidget->layout());
    if ( !layout )
    {
        return;
    }
    ui->showAllOffers->setIcon ( KIcon(QLatin1String( m_showAllOffers ? "arrow-up-double" : "arrow-down-double" )) );
    ui->showAllOffers->setVisible ( m_offerWidgets.size() > 1 );
    foreach ( OfferWidget* widget, m_offerWidgets )
    {
        layout->removeWidget ( widget );
        widget->hide();
    }
    if ( m_showAllOffers )
    {
        for ( int i = 0; i < m_offerWidgets.size(); ++i )
        {
            layout->addWidget ( m_offerWidgets[i], i+1, 0 );
            m_offerWidgets[i]->show();
        }
    }
    else
    {
        layout->addWidget ( m_offerWidgets.last(), 1, 0 );
        m_offerWidgets.last()->show();
    }
    ui->notificationWidget->show();
}





#include "knightsview.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
