/*
    This file is part of Knights, a chess board for KDE SC 4.
    Copyright 2009-2010  Miha Čančula <miha.cancula@gmail.com>

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

using namespace Knights;

KnightsView::KnightsView ( QWidget *parent )
        : QWidget ( parent )
{
    ui.setupUi ( this );
    m_board = 0;
    settingsChanged();
}

KnightsView::~KnightsView()
{

}

void KnightsView::setupBoard ( Protocol* protocol )
{
    m_board = new Board ( this );
    ui.canvas->setScene ( m_board );
    resizeScene();

    if ( protocol )
    {
        connect ( m_board, SIGNAL ( pieceMoved ( Move ) ), protocol, SLOT ( move ( Move ) ) );
        connect ( protocol, SIGNAL ( pieceMoved ( Move ) ), m_board, SLOT ( movePiece ( Move ) ) );
        m_board->setPlayerColors ( QList<Color>() << protocol->playerColor() );
    }
    else
    {
        m_board->setPlayerColors ( QList<Color>() << White << Black );
    }

    if ( protocol && protocol->supportedFeatures() & Protocol::GameOver )
    {
        connect ( protocol, SIGNAL ( gameOver ( Color ) ), SLOT ( gameOver ( Color ) ) );
    }
    else
    {
        connect ( m_board, SIGNAL ( gameOver ( Color ) ), SLOT ( gameOver ( Color ) ) );
    }

    connect ( m_board, SIGNAL ( activePlayerChanged ( Color ) ), SIGNAL ( activePlayerChanged ( Color ) ) );
    connect ( m_board, SIGNAL ( displayedPlayerChanged ( Color ) ), SIGNAL ( displayedPlayerChanged ( Color ) ) );

    connect ( m_board, SIGNAL ( centerChanged ( QPointF ) ), this, SLOT ( centerView ( QPointF ) ) );

}

void KnightsView::gameOver ( Color winner )
{
    kDebug() << "Received gameOver() from " << sender()->metaObject()->className();
    QString text;
    QString caption;
    if ( winner == NoColor )
    {
        text = i18n ( "The game ended in a draw" );
        KMessageBox::information ( this, text );
    }
    else
    {
        text = i18n ( "The %1 player won.", colorName ( winner ) );
        if ( m_board->playerColors().contains ( winner ) )
        {
            KMessageBox::information ( this, text, i18n ( "Congratulations!" ) );
        }
        else
        {
            KMessageBox::sorry ( this, text );
        }
    }
    emit gameNew();
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
    if ( ui.canvas && m_board )
    {
        m_board->setSceneRect ( ui.canvas->contentsRect() );
        m_board->updateGraphics();
        ui.canvas->setTransform ( QTransform() );
    }
}

void KnightsView::setPaused ( bool paused )
{
    m_board->setPaused ( paused );
}

QString KnightsView::colorName ( Color color )
{
    switch ( color )
    {
        case White:
            return i18n ( "White" );
        case Black:
            return i18n ( "Black" );
        default:
            return QString();
    }
}

QString KnightsView::pieceTypeName ( PieceType type )
{
    switch ( type )
    {
        case Pawn:
            return i18n ( "Pawn" );
        case Rook:
            return i18n ( "Rook" );
        case Knight:
            return i18n ( "Knight" );
        case Bishop:
            return i18n ( "Bishop" );
        case Queen:
            return i18n ( "Queen" );
        case King:
            return i18n ( "King" );
        default:
            return QString();
    }
}

void KnightsView::centerView ( const QPointF& center )
{
    if ( ui.canvas )
    {
        ui.canvas->centerOn ( center );
    }
}


#include "knightsview.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
