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
#include "rules/chessrules.h"
#include "settings.h"

#include <KGameTheme>
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

#include "core/item.h"

#if defined HAVE_RENDER
    #include "KGameRenderer"    
#else
    #include "QtSvg/QSvgRenderer"
    #include "kgametheme.h"
#endif

using namespace Knights;

const qreal tileZValue = 0.0;
const qreal pieceZValue = 1.0;
const qreal markerZValue = 2.0;
const qreal dragZValue = 3.0;

Board::Board ( QObject* parent ) : QGraphicsScene ( parent )
{
    QString themeName = Settings::theme();
    #if defined HAVE_RENDER
        renderer = new KGameRenderer(themeName);
    #else
        renderer = new QSvgRenderer;
        theme = new KGameTheme;
    #endif
    setRuleSet ( new ChessRules );
    updateTheme();
    m_currentPlayer = White;
    m_paused = false;
    
    connect ( this, SIGNAL(sceneRectChanged(QRectF)), SLOT(updateGraphics()) );
    connect ( this, SIGNAL(displayedPlayerChanged(Color)), SLOT(displayPlayer(Color)));
}

Board::~Board()
{

}

void Board::addPiece ( PieceType type, Color color, const Knights::Pos& pos )
{
    Piece* t_piece = new Piece ( renderer, type, color, this );
    if ( Settings::animationSpeed() != Settings::EnumAnimationSpeed::Instant )
    {
        t_piece->setPos ( mapToScene ( Pos ( ( pos.first > 4 ) ? 5 : 4, ( pos.second > 4 ) ? 5 : 4 ) ) );
    }
    t_piece->setZValue ( pieceZValue );
    m_grid.insert ( pos, t_piece );
}

void Board::movePiece ( Move m, bool changePlayer )
{
    if ( !m_grid.contains ( m.from() ) || m.to() == m.from() )
    {
        kWarning() << "Invalid move" << m.from() << m.to();
        return;
    }
    m_rules->checkSpecialFlags ( &m );
    centerOnPos ( m_grid.value ( m.from() ), m.to() );
    delete m_grid.value ( m.to(), 0 ); // It's safe to call 'delete 0'
    m_grid.insert ( m.to(), m_grid.take ( m.from() ) );
    addMarker ( m.from(), Motion );
    addMarker ( m.to(), Motion );
    if ( m.flags() & Move::EnPassant )
    {
        foreach ( const Pos& p, m.additionalCaptures() )
        {
            delete m_grid.value ( p, 0 );
            m_grid.remove ( p );
        }
    }
    if ( m.flags() & Move::Castle )
    {
        foreach ( const Move& additionalMove, m.additionalMoves() )
        {
            movePiece ( additionalMove, false );
        }
    }
    m_rules->moveMade ( m );
    Color winner = m_rules->winner();
    if ( winner != NoColor || !m_rules->hasLegalMoves ( oppositeColor ( m_currentPlayer ) ) )
    {
        emit gameOver ( winner );
        kDebug() << winner;
    }
    else if ( changePlayer )
    {
        changeCurrentPlayer();
    }
}

void Board::populate()
{
    const BoardState pieces = m_rules->startingPieces ();
    foreach ( const Pos& pos, pieces.keys() )
    {
        addPiece ( pieces[pos].second, pieces[pos].first, pos );
    }
    const QString whiteTileKey = "WhiteTile";
    const QString blackTileKey = "BlackTile";
    for (int i = 1; i < 9; ++i)
    {
        for (int j = 1; j < 9; ++j)
        {
            Item* tile;
            if ( (i + j) % 2 ) 
            {
                tile = new Item( renderer, whiteTileKey, this );
            }
            else
            {
                tile = new Item( renderer, blackTileKey, this );
            }
            m_tiles.insert(Pos(i,j), tile);
        }
    }
}

void Board::setRuleSet ( Rules* rules )
{
    m_rules = rules;
    m_rules->setGrid ( &m_grid );
}

void Board::mousePressEvent ( QGraphicsSceneMouseEvent* e )
{
    if ( m_paused || !m_playerColors.contains ( m_currentPlayer ) )
    {
        // It is not the human player's turn
        e->ignore();
        return;
    }
    Piece* d_piece = pieceAt ( e->scenePos() );
    if ( !d_piece || d_piece->color() != m_currentPlayer )
    {
        // The piece doesn't belong to the player whose turn it is
        e->ignore();
        return;
    }
    else
    {
        Pos t_pos = mapFromScene ( e->scenePos() );
        QList<Move> t_legalMoves = m_rules->legalMoves ( t_pos );
        if ( t_legalMoves.isEmpty() )
        {
            e->ignore();
            return;
        }
        d_piece->setZValue ( dragZValue );
        foreach ( const Move& t_move, t_legalMoves )
        {
            addMarker ( t_move.to(), LegalMove );
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
    foreach ( QGraphicsItem* marker, markers )
    {
        removeItem ( marker );
        delete marker;
    }
    markers.clear();

    if ( e->mimeData()->hasText() )
    {
        QStringList list = e->mimeData()->text().split ( '_' );
        if ( list.size() < 2 )
        {
            e->ignore();
            return;
        }
        Pos from ( list.first().toInt(), list.last().toInt() );
        Pos to = mapFromScene ( e->scenePos() );
        Move move ( from, to );
        if ( !m_rules->legalMoves ( from ).contains ( move ) )
        {
            centerOnPos ( m_draggedItem, m_grid.key ( m_draggedItem ) );
        }
        else
        {
            movePiece ( move );
            emit pieceMoved ( move );
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
    if ( !m_draggedItem )
    {
        return;
    }
    qreal x = e->scenePos().x() - m_draggedPos.x();
    qreal y = e->scenePos().y() - m_draggedPos.y();

    m_draggedItem->moveBy ( x, y );
    m_draggedPos += QPointF ( x, y );
}

Piece* Board::pieceAt ( QPointF point )
{
    return m_grid.value ( mapFromScene ( point ), 0 );
}

Pos Board::mapFromScene ( QPointF point )
{
    Pos pos;
    pos.first = ( point.x() - m_boardRect.left() ) / m_tileSize + 1;
    pos.second = 1 - ( point.y() - m_boardRect.bottom() ) / m_tileSize;
    if ( m_displayedPlayer != White )
    {
        pos = Pos ( 9, 9 ) - pos;
    }
    return pos;
}

QPointF Board::mapToScene ( Pos pos )
{
    if ( m_displayedPlayer != White )
    {
        pos = Pos ( 9, 9 ) - pos;
    }
    QPointF point;
    point.setX ( m_boardRect.left() + ( pos.first - 1 ) * m_tileSize );
    point.setY ( m_boardRect.bottom() - pos.second * m_tileSize );
    return point;
}

void Board::centerOnPos ( Item* item, const Knights::Pos& pos, bool animated )
{
    QSize rectSize = item->renderSize();
    QPointF slide = QPointF(rectSize.width(), rectSize.height()) - QPointF ( m_tileSize, m_tileSize );
    QPointF endPos = mapToScene ( pos );
#if QT_VERSION >= 0x040600
    if ( !animated || Settings::animationSpeed() == Settings::EnumAnimationSpeed::Instant )
    {
        item->setPos ( endPos );
    }
    else
    {
        // TODO: Is there a better way to determine duration than *= sqrt(manhattanLenght());
        int duration = 50;
        switch ( Settings::animationSpeed() )
        {
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
        duration *= qSqrt ( QPointF ( item->pos() - endPos ).manhattanLength() / m_tileSize );
        Piece* t_piece = qgraphicsitem_cast<Piece*> ( item );
        QPropertyAnimation* anim = new QPropertyAnimation ( t_piece, "pos" );
        anim->setDuration ( duration );
        anim->setEasingCurve ( QEasingCurve::InOutCubic );
        anim->setStartValue ( item->pos() );
        anim->setEndValue ( endPos );
        anim->start ( QAbstractAnimation::DeleteWhenStopped );
    }
#else
    Q_UNUSED ( animated );
    item->setPos ( endPos );
#endif

}

bool Board::isInBoard ( const Knights::Pos& pos )
{
    return pos.first > 0 && pos.first < 9 && pos.second > 0 && pos.second < 9;
}

QList< Color > Board::playerColors()
{
    return m_playerColors;
}

void Board::setPlayerColors ( const QList< Color >& colors )
{
    if ( colors.isEmpty() )
    {
        qDebug() << "The 'Two computers one board' feature not yet implemented";
        return;
    }
    m_playerColors = colors;
    if ( m_playerColors.contains ( m_currentPlayer ) )
    {
        m_displayedPlayer = m_currentPlayer;
    }
    else
    {
        m_displayedPlayer = m_playerColors.first();
    }
    populate();
}

void Board::changeCurrentPlayer()
{
    m_currentPlayer = oppositeColor ( m_currentPlayer );
    if ( m_playerColors.contains ( m_currentPlayer ) )
    {
        if ( m_displayedPlayer != m_currentPlayer )
        {
            m_displayedPlayer = m_currentPlayer;
            emit displayedPlayerChanged ( m_displayedPlayer );
        }
        m_displayedPlayer = m_currentPlayer;
    }
    emit activePlayerChanged ( m_currentPlayer );
    // TODO: Stop & start clocks at this point
}

void Board::setCurrentColor ( Color color )
{
    if ( m_currentPlayer != color )
    {
        m_currentPlayer = color;
    }
}


void Board::addMarker ( const Knights::Pos& pos, MarkerType type )
{
    QString key;
    switch (type)
    {
        case LegalMove:
            key = "Marker";
            break;
        case Danger:
            key = "Danger";
            break;
        case Motion:
            key = "Motion";
            break;
    }
    Item* marker = new Item ( renderer, key, this);
    centerOnPos ( marker, pos, false );
    marker->setRenderSize ( QSizeF(m_tileSize, m_tileSize).toSize() );
    marker->setZValue ( markerZValue );
    markers.insert(pos, marker);
}

void Board::setPaused ( bool paused )
{
    m_paused = paused;
}


void Board::updateTheme()
{
    QString themeName = Settings::theme();
    #if defined HAVE_RENDER
        renderer->setTheme( themeName );
    #else
        if (!theme->load( themeName ) )
        {
            theme->loadDefault();
        }
        kDebug() << theme->graphics();
        renderer->load(theme->graphics());
    #endif
    updateGraphics();
}

void Board::updateGraphics()
{
    m_tileSize = floor ( qMin(sceneRect().height(), sceneRect().width()) / 8 * 0.95);
    qreal sideMargin = sceneRect().width() - 8 * m_tileSize;
    qreal topMargin = sceneRect().height() - 8 * m_tileSize;
    m_boardRect = QRectF ( sideMargin / 2, topMargin / 2, m_tileSize * 8, m_tileSize * 8 );
    renderer->setViewBox(m_boardRect);
    kDebug() << sceneRect() << m_boardRect;
    QSize tSize = QSizeF(m_tileSize, m_tileSize).toSize();
    foreach ( Piece* p, m_grid )
    {
        p->setRenderSize ( tSize );
        centerOnPos( p, m_grid.key( p ) );
    }
    foreach ( const Pos& p, m_tiles.keys() )
    {
        Item* t = m_tiles[p];
        t->setRenderSize ( tSize );
        centerOnPos( t, m_tiles.key( t ), false );
    }
    if (!markers.isEmpty())
    {
    foreach ( Item* t, markers )
    {        
        t->setRenderSize ( tSize );
        centerOnPos( t, markers.key( t ), false );
    }
    }
    
    emit centerChanged( QPointF( 4 * m_tileSize, 4 * m_tileSize ) );
}

void Board::displayPlayer(Color color)
{
    kDebug() << color;
    foreach ( Piece* p, m_grid )
    {
        centerOnPos( p, m_grid.key( p ) );
    }
    foreach ( Item* i, markers )
    {
        centerOnPos( i, markers.key ( i ) );
    }
}


#include "board.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
