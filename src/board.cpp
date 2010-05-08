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

#include "board.h"

#include "core/pos.h"
#include "core/move.h"
#include "chessrules.h"
#include "settings.h"

#include <KGameTheme>
#include <KSvgRenderer>
#include <KDebug>

#include <QtCore/QMap>
#include <QtGui/QDropEvent>
#include <QtSvg/QGraphicsSvgItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsView>
#include <QtCore/qmath.h>

#if QT_VERSION >= 0x040600
#include <QtCore/QPropertyAnimation>
#endif

using namespace Knights;

const QString rowNames = QString::fromAscii ( "abcdefgh" );
const qreal tileZValue = 0.0;
const qreal pieceZValue = 1.0;
const qreal markerZValue = 2.0;
const qreal dragZValue = 3.0;

Board::Board ( QObject* parent ) : QGraphicsScene ( parent )
{

    m_draggedItem = 0;
    setRuleSet ( new ChessRules );
    theme = new KGameTheme;
    updateTheme();
    m_currentPlayer = Piece::White;
    m_paused = false;
    //  updateBoard();
}

Board::~Board()
{

}

void Board::addPiece ( Piece::PieceType type, Piece::Color color, Pos pos )
{
    Piece* t_piece = new Piece ( type, color );
    if ( Settings::animationSpeed() != Settings::EnumAnimationSpeed::Instant ) {
        t_piece->setPos ( mapToScene ( Pos ( ( pos.first > 4 ) ? 5 : 4, ( pos.second > 4 ) ? 5 : 4 ) ) );
    }
    QString id;
    switch ( color ) {
    case Piece::White:
        id.append ( 'W' );
        break;
    case Piece::Black:
        id.append ( 'B' );
        break;
    default:
        break;
    }
    id.append ( '_' );
    switch ( type ) {
    case Piece::Pawn:
        id.append ( "PAWN" );
        break;
    case Piece::Rook:
        id.append ( "ROOK" );
        break;
    case Piece::Knight:
        id.append ( "KNIGHT" );
        break;
    case Piece::Bishop:
        id.append ( "BISHOP" );
        break;
    case Piece::Queen:
        id.append ( "QUEEN" );
        break;
    case Piece::King:
        id.append ( "KING" );
        break;
    }
    // m_elementIdMap.insert(t_piece,id);
    QSizeF t_size = svg->boundsOnElement ( id ).size();
    qreal scale = m_tileSize / qMax ( t_size.width(), t_size.height() );
    QTransform transform = QTransform().scale ( scale, scale );
    t_piece->setTransform ( transform );
    t_piece->setElementId ( id );
    t_piece->setSharedRenderer ( svg );
    t_piece->setZValue ( pieceZValue );
    m_grid.insert ( pos,t_piece );
    addItem ( t_piece );
    centerOnPos ( t_piece, pos );
}

void Board::movePiece ( Move m, bool changePlayer )
{
    kDebug();
    if ( !m_grid.contains ( m.from() ) || m.to() == m.from() ) {
        kWarning() << "Invalid move" << m.from() << m.to();
        return;
    }
    m_rules->checkSpecialFlags ( &m );
    centerOnPos ( m_grid.value ( m.from() ), m.to() );
    delete m_grid.value ( m.to(),0 ); // It's safe to call 'delete 0'
    m_grid.insert ( m.to(),m_grid.take ( m.from() ) );
    if ( m.flags() & Move::EnPassant ) {
        foreach ( const Pos& p, m.additionalCaptures() ) {
            delete m_grid.value ( p, 0 );
            m_grid.remove ( p );
        }
    }
    if ( m.flags() & Move::Castle ) {
        foreach ( const Move& additionalMove, m.additionalMoves() ) {
            movePiece ( additionalMove, false );
        }
    }
    m_rules->moveMade ( m );
    Piece::Color winner = m_rules->winner();
    if ( winner != Piece::NoColor || !m_rules->hasLegalMoves(Piece::oppositeColor(m_currentPlayer))) {
        emit gameOver ( winner );
        kDebug() << winner;
    }
    else if ( changePlayer ) {
        changeCurrentPlayer();
    }
}

void Board::populate()
{
    QMap<Pos,Piece::PieceType> pieces = m_rules->startingPieces ( Piece::White );
    foreach ( const Pos& pos, pieces.keys() ) {
        addPiece ( pieces.value ( pos ), Piece::White, pos );
    }
    pieces = m_rules->startingPieces ( Piece::Black );
    foreach ( const Pos& pos, pieces.keys() ) {
        addPiece ( pieces.value ( pos ), Piece::Black, pos );
    }
}

void Board::setRuleSet ( Rules* rules )
{
    m_rules = rules;
    m_rules->setGrid ( &m_grid );
}

void Board::mousePressEvent ( QGraphicsSceneMouseEvent* e )
{
    if ( m_paused || !m_playerColors.contains ( m_currentPlayer ) ) {
        // It is not the human player's turn
        e->ignore();
        return;
    }
    Piece* d_piece = pieceAt ( e->scenePos() );
    if ( !d_piece || d_piece->color() != m_currentPlayer ) {
        // The piece doesn't belong to the player whose turn it is
        e->ignore();
        return;
    } else {
        Pos t_pos = mapFromScene ( e->scenePos() );
        QList<Move> t_legalMoves = m_rules->legalMoves(t_pos);
        if (t_legalMoves.isEmpty())
        {
            e->ignore();
            return;
        }
        d_piece->setZValue ( dragZValue );
        foreach ( const Move& t_move, t_legalMoves ) {
            addMarker ( t_move.to() );
        }
        QDrag* drag = new QDrag ( e->widget() );
        QString posText = QString::number ( t_pos.first ) + '_' + QString::number ( t_pos.second );
        QMimeData* data = new QMimeData;
        data->setText ( posText );
        m_draggedItem = d_piece;
        m_draggedPos = e->scenePos();
        m_dragStartPos = m_draggedPos;
        drag->setMimeData ( data );
        drag->start ( Qt::MoveAction );
    }
}

void Board::dropEvent ( QGraphicsSceneDragDropEvent* e )
{
    foreach ( QGraphicsSvgItem* marker, m_legalMarkers ) {
        removeItem ( marker );
        delete marker;
    }
    m_legalMarkers.clear();

    if ( e->mimeData()->hasText() ) {
        QStringList list = e->mimeData()->text().split ( '_' );
        if ( list.size() < 2 ) {
            e->ignore();
            return;
        }
        Pos from ( list.first().toInt(), list.last().toInt() );
        Pos to = mapFromScene ( e->scenePos() );
        Move move ( from, to );
        if ( !m_rules->legalMoves ( from ).contains ( move ) ) {
            centerOnPos ( m_draggedItem, m_grid.key ( m_draggedItem ) );
        } else {
            movePiece ( move );
            emit pieceMoved(move);
        }
        m_draggedItem->setZValue ( pieceZValue );
        m_draggedItem = 0;
    }
}

void Board::dragEnterEvent ( QGraphicsSceneDragDropEvent* e )
{
    e->accept();
}

void Board::dragMoveEvent ( QGraphicsSceneDragDropEvent* e )
{
    if ( !m_draggedItem ) {
        return;
    }
    qreal x = e->scenePos().x() - m_draggedPos.x();
    qreal y = e->scenePos().y() - m_draggedPos.y();

    m_draggedItem->moveBy ( x,y );
    m_draggedPos += QPointF ( x,y );
}

Piece* Board::pieceAt ( QPointF point )
{
    return m_grid.value ( mapFromScene ( point ), 0 );
}

Pos Board::mapFromScene ( QPointF point )
{
    Pos pos;
    pos.first = ( point.x() - sceneRect().left() ) * 8 / height() + 1;
    pos.second = 1 - ( point.y() - sceneRect().bottom() ) *8/width();
    if ( m_displayedPlayer != Piece::White ) {
        pos = Pos ( 9,9 ) - pos;
    }
    return pos;
}

QPointF Board::mapToScene ( Pos pos )
{
    if ( m_displayedPlayer != Piece::White ) {
        pos = Pos ( 9,9 ) - pos;
    }
    QPointF point;
    point.setX ( m_boardRect.left() + ( pos.first - 1 ) * m_boardRect.width() / 8 );
    point.setY ( m_boardRect.bottom() - pos.second * m_boardRect.height() / 8 );
    return point;
}

void Board::centerOnPos ( QGraphicsItem* item, Pos pos, bool animated )
{
    QRectF rect = item->transform().mapRect ( item->boundingRect() );
    QPointF slide = rect.bottomRight() - rect.topLeft() - QPointF ( m_tileSize, m_tileSize );
    QPointF endPos = mapToScene ( pos ) - slide/2;
#if QT_VERSION >= 0x040600
    if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant || m_grid.keys ( ( Piece* ) item ).isEmpty() ) {
        item->setPos ( endPos );
    } else {
        // TODO: Is there a better way to determine duration than *= sqrt(manhattanLenght());
        int duration = 50;
        switch ( Settings::animationSpeed() ) {
        case Settings::EnumAnimationSpeed::Fast:
            duration = 200;
            break;
        case Settings::EnumAnimationSpeed::Normal:
            duration = 500;
            break;
        case Settings::EnumAnimationSpeed::Slow:
            duration = 1000;
            break;
        default:
            break;
        }
        duration *= qSqrt ( QPointF ( item->pos() - endPos ).manhattanLength() /m_tileSize );
        Piece* t_piece = qgraphicsitem_cast<Piece*> ( item );
        QPropertyAnimation* anim = new QPropertyAnimation ( t_piece, "pos" );
        anim->setDuration ( duration );
        anim->setEasingCurve ( QEasingCurve::InOutCubic );
        anim->setStartValue ( item->pos() );
        anim->setEndValue ( endPos );
        anim->start ( QAbstractAnimation::DeleteWhenStopped );
    }
#else
    Q_UNUSED(animated);
    item->setPos ( endPos );
#endif

}

QChar Board::row ( int num )
{
    if ( num > 0 && num << 9 ) {
        return rowNames[num - 1];
    } else {
        return QChar ( 'r' );
    }
}

int Board::numFromRow ( QChar row )
{
    return rowNames.indexOf ( row ) + 1;
}

bool Board::isInBoard ( Pos pos )
{
    return pos.first > 0 && pos.first < 9 && pos.second > 0 && pos.second < 9;
}

QList< Piece::Color > Board::playerColors()
{
    return m_playerColors;
}

void Board::setPlayerColors ( QList< Piece::Color > colors )
{
    if ( colors.isEmpty() ) {
        qDebug() << "The 'Two computers one board' feature not yet implemented";
        return;
    }
    m_playerColors = colors;
    if ( m_playerColors.contains ( m_currentPlayer ) ) {
        m_displayedPlayer = m_currentPlayer;
    } else {
        m_displayedPlayer = m_playerColors.first();
    }
    repaintBoard();
    populate();
}

void Board::changeCurrentPlayer()
{
    m_currentPlayer = Piece::oppositeColor( m_currentPlayer );
    if ( m_playerColors.contains ( m_currentPlayer ) ) {
        if ( m_displayedPlayer != m_currentPlayer ) {
            m_displayedPlayer = m_currentPlayer;
            emit displayedPlayerChanged(m_displayedPlayer);
            repaintBoard();
        }
        m_displayedPlayer = m_currentPlayer;
    }
    emit activePlayerChanged(m_currentPlayer);
    // TODO: Stop & start clocks at this point
}

void Board::setCurrentColor ( Piece::Color color )
{
    if ( m_currentPlayer != color ) {
        m_currentPlayer = color;
        repaintBoard();
    }
}


void Board::addMarker ( const Knights::Pos& pos )
{
    QGraphicsSvgItem* marker = new QGraphicsSvgItem();
    marker->setElementId ( QString::fromAscii ( "MARKER" ) );
    marker->setSharedRenderer ( svg );
    marker->setTransform ( markerTransform );
    centerOnPos ( marker, pos, false );
    marker->setZValue ( markerZValue );
    m_legalMarkers << marker;
    addItem ( marker );
}

void Board::setPaused(bool paused)
{
    m_paused = paused;
}


void Board::updateTheme()
{
    if ( !theme->load ( Settings::theme() ) ) {
        theme->loadDefault();
    }
    svg = new KSvgRenderer ( this );
    svg->load ( theme->graphics() );
    QSizeF tileSize = svg->boundsOnElement ( "W_TILE" ).size();
    m_boardRect = QRectF ( QPointF ( 0.0,0.0 ),8 * (tileSize - QSizeF(1.0,1.0)));
    m_tileSize = qMax ( tileSize.width(), tileSize.height() ); // - 1.0; // Trying to fix visible lines between tiles
    QSizeF markerSize = svg->boundsOnElement ( "MARKER" ).size();
    qreal markerLen =  qMax ( markerSize.width(), markerSize.height() );
    markerTransform = QTransform().scale ( m_tileSize / markerLen, m_tileSize / markerLen );
    foreach(Piece* p, m_grid)
    {
        removeItem(p);
        addPiece(p->pieceType(), p->color(), m_grid.key(p));
        delete p;
    }
    repaintBoard();
}

void Board::repaintBoard()
{
    // This function is NOT for theme change
    kDebug();
    // Move the pieces if necessary
    foreach ( QGraphicsItem* item, items() ) {
        QGraphicsSvgItem* svgItem = qgraphicsitem_cast<QGraphicsSvgItem*> ( item );
        if ( svgItem ) {
            if ( !m_grid.keys ( ( Piece* ) item ).isEmpty() ) {
                // If it's a piece, move it.
                centerOnPos ( item, m_grid.key ( ( Piece* ) item ) );
            } else {
                // If it's not a piece, delete it.
                // Tiles will be rebuilt later.
                removeItem ( item );
                delete item;
            }
        }
    }
    // Load and draw the tiles
    for ( int i = 1; i < 9; ++i ) {
        for ( int j = 1; j < 9; ++j ) {
            QGraphicsSvgItem* tile = new QGraphicsSvgItem();
            tile->setSharedRenderer ( svg );
            QString id = QString::fromAscii ( "%1_TILE" ).arg ( ( ( i+j ) % 2 == 0 ) ? 'B' : 'W' );
            tile->setElementId ( id );
            tile->setPos ( mapToScene ( Pos ( i,j ) ) );
            m_elementIdMap.insert ( tile,id );
            tile->setZValue ( tileZValue );
            addItem ( tile );
        }
    }
    setSceneRect ( m_boardRect );
}

#include "board.moc"
