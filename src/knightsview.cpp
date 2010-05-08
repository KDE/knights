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

KnightsView::KnightsView(QWidget *parent)
        : QWidget(parent)
{
    ui.setupUi(this);
    m_board = 0;
    settingsChanged();
}

KnightsView::~KnightsView()
{

}

void KnightsView::setupBoard(Protocol* protocol, QList<Piece::Color> playerColors)
{
    m_board = new Board(this);
    if (protocol)
    {
        connect(m_board, SIGNAL(pieceMoved(Move)), protocol, SLOT(move(Move)));
        connect(protocol, SIGNAL(pieceMoved(Move)), m_board, SLOT(movePiece(Move)));
        connect(protocol, SIGNAL(gameOver(Piece::Color)), SLOT(gameOver(Piece::Color)));
    }
    connect(m_board, SIGNAL(gameOver(Piece::Color)), SLOT(gameOver(Piece::Color)));
    connect(m_board, SIGNAL(sceneRectChanged(QRectF)), SLOT(resizeScene()));
    m_board->setPlayerColors(playerColors);
    ui.canvas->setScene(m_board);
    kDebug() << "Fitting in view";
    resizeScene();

    connect(m_board, SIGNAL(activePlayerChanged(Piece::Color)), SIGNAL(activePlayerChanged(Piece::Color)));
    connect(m_board, SIGNAL(displayedPlayerChanged(Piece::Color)), SIGNAL(displayedPlayerChanged(Piece::Color)));
}


void KnightsView::gameOver(Piece::Color winner)
{
  QString text;
  QString caption;
  if (winner == Piece::NoColor)
  {
    text = i18n("The game ended in a draw");
    KMessageBox::information(this, text);
  }
  else
  {
    text = i18n("The %1 player won.", colorName(winner));
    if (m_board->playerColors().contains(winner))
    {
      KMessageBox::information(this, text, i18n("Congratulations!"));
    }
    else
    {
      KMessageBox::sorry(this, text);
    }
  }
  emit gameNew();
}

void KnightsView::settingsChanged()
{
    emit signalChangeStatusbar( i18n("Settings changed") );
	if (m_board)
	{
		m_board->updateTheme();
	}
}

void KnightsView::resizeEvent(QResizeEvent* e)
{
    Q_UNUSED(e)
    resizeScene();
}

void KnightsView::resizeScene()
{
  if (ui.canvas && m_board)
  {
    ui.canvas->fitInView(m_board->sceneRect(), Qt::KeepAspectRatio);
  }
}

void KnightsView::setPaused(bool paused)
{
  m_board->setPaused(paused);
}

QString KnightsView::colorName(Piece::Color color)
{
  switch (color)
  {
    case Piece::White:
      return i18n("White");
    case Piece::Black:
      return i18n("Black");
    default:
      return QString();
  }
}

QString KnightsView::pieceTypeName(Piece::PieceType type)
{
  switch (type)
  {
    case Piece::Pawn:
      return i18n("Pawn");
    case Piece::Rook:
      return i18n("Rook");
    case Piece::Knight:
      return i18n("Knight");
    case Piece::Bishop:
      return i18n("Bishop");
    case Piece::Queen:
      return i18n("Queen");
    case Piece::King:
      return i18n("King");
    default:
      return QString();
  }
}



#include "knightsview.moc"
