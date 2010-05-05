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

#ifndef KNIGHTSVIEW_H
#define KNIGHTSVIEW_H

#include "board.h"

#include "ui_knightsview_base.h"

#include <QtGui/QWidget>

class QPainter;
class KUrl;

namespace Knights
{
class Protocol;

/**
 * This is the main view class for Knights.  Most of the non-menu,
 * non-toolbar, and non-statusbar (e.g., non frame) GUI code should go
 * here.
 *
 * @short Main view
 * @author %{AUTHOR} <%{EMAIL}>
 * @version %{VERSION}
 */

class KnightsView : public QWidget, public Ui::knightsview_base
{
    Q_OBJECT
public:
    /**
     * Default constructor
     */
    KnightsView(QWidget *parent);
    /**
     * Destructor
     */
    virtual ~KnightsView();
    void setupBoard(Knights::Protocol* protocol = 0, QList< Piece::Color > playerColors = QList<Piece::Color>());
    void setPaused(bool paused);

 private:
    Ui::knightsview_base ui;
    Board* m_board;
    Protocol* m_protocol;
    
    static QString pieceTypeName(Piece::PieceType);
    static QString colorName(Piece::Color);

signals:
    /**
     * Use this signal to change the content of the statusbar
     */
    void signalChangeStatusbar(const QString& text);

    /**
     * Use this signal to change the content of the caption
     */
    void signalChangeCaption(const QString& text);

    void gameNew();

    //Signals from board:

    void activePlayerChanged(Piece::Color);
    void displayedPlayerChanged(Piece::Color);

private slots:
    void settingsChanged();
    void resizeScene();
    void gameOver(Piece::Color winner);

  protected:
    virtual void resizeEvent(QResizeEvent* e);
};

}

#endif // KnightsVIEW_H
