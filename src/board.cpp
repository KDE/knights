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
  #define HAVE_ANIMATIONS
  #include <QtCore/QPropertyAnimation>
#endif

#include "core/item.h"
#include "core/renderer.h"

using namespace Knights;

const qreal tileZValue = 0.0;
const qreal pieceZValue = 1.0;
const qreal motionMarkerZValue = 1.5;
const qreal legalMarkerZValue = 2.0;
const qreal dragZValue = 3.0;

const QString whiteTileKey = "WhiteTile";
const QString blackTileKey = "BlackTile";
const QString legalMarkerKey = "Marker";
const QString motionMarkerKey = "Motion";
const QString dangerMarkerKey = "Danger";

const QString tbBorderKey = "TopBottomBorder";
const QString lrBorderKey = "LeftRightBorder";

Board::Board ( QObject* parent ) : QGraphicsScene ( parent )
{
    renderer = new Renderer( Settings::theme() );
    setRuleSet ( new ChessRules );
    updateTheme();
    m_currentPlayer = White;
    m_paused = false;
    
    connect ( this, SIGNAL(sceneRectChanged(QRectF)), SLOT(updateGraphics()) );
    connect ( this, SIGNAL(displayedPlayerChanged(Color)), SLOT(changeDisplayedPlayer()) );
}

Board::~Board()
{
    qDeleteAll(m_grid);
    qDeleteAll(m_tiles);
    qDeleteAll(markers);
}

void Board::addPiece ( PieceType type, Color color, const Knights::Pos& pos )
{
    Piece* t_piece = new Piece ( renderer, type, color, this, pos );
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
    if ( m_playerColors.contains ( oppositeColor ( m_currentPlayer )))
    {
        // We only display motion and danger markers if the next player is a human
        if (Settings::showMotion())
        {
            addMarker ( m.from(), Motion );
            addMarker ( m.to(), Motion );
        }
        if (Settings::showDanger())
        {
            bool check = false;
            foreach ( Piece* piece, m_grid )
            {
                if ( piece->color() == m_currentPlayer && m_rules->isAttacking ( piece->boardPos() ) )
                {
                    check = true;
                    addMarker( piece->boardPos(), Danger );
                }
            }
            if ( check )
            {
                foreach ( Piece* piece, m_grid )
                {
                    if ( piece->color() != m_currentPlayer && piece->pieceType() == King )
                    {
                        addMarker( piece->boardPos(), Danger );
                    }
                }
            }   
        }
    }
    if ( m.flags() & Move::EnPassant )
    {
        kDebug() << m.additionalCaptures();
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
        kDebug() << "Winner: " << winner;
        emit gameOver ( winner );
    }
    if ( changePlayer )
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
    updateGraphics();
}

void Board::addTiles()
{
    if (!m_tiles.isEmpty())
    {
        kWarning() << "Tiles are already present, delete them first";
        return;
    }
    for (int i = 1; i < 9; ++i)
    {
        for (int j = 1; j < 9; ++j)
        {
            QString key = ((i+j) % 2 == 0) ? blackTileKey : whiteTileKey;
            Item* tile = new Item ( renderer, key, this, Pos(i,j) );
            tile->setZValue(tileZValue);
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
        // The active player clicked on his/her own piece
        qDeleteAll(markers);
        markers.clear();
        Pos t_pos = mapFromScene ( e->scenePos() );
        QList<Move> t_legalMoves = m_rules->legalMoves ( t_pos );
        if ( t_legalMoves.isEmpty() )
        {
            e->ignore();
            return;
        }
        d_piece->setZValue ( dragZValue );
        if (Settings::showMarker())
        {
            foreach ( const Move& t_move, t_legalMoves )
            {
                addMarker ( t_move.to(), LegalMove );
            }
        }
        QDrag* drag = new QDrag ( e->widget() );
        QString posText = QString::number ( t_pos.first ) + '_' + QString::number ( t_pos.second );
        QMimeData* data = new QMimeData;
        data->setText ( posText );
        m_draggedItem = d_piece;
        m_draggedPos = e->scenePos();
        m_dragStartPos = m_draggedPos;
        drag->setMimeData ( data );
        drag->exec();
    }
}

void Board::dropEvent ( QGraphicsSceneDragDropEvent* e )
{
    qDeleteAll(markers);
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

void Board::centerOnPos ( Knights::Item* item, const Knights::Pos& pos, bool animated )
{
    item->setBoardPos ( pos );
    centerOnPos( item, animated );
}

void Board::centerOnPos(Knights::Item* item, bool animated)
{
    QPointF endPos = mapToScene ( item->boardPos() );
#if defined HAVE_ANIMATIONS
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
            emit displayedPlayerChanged(m_displayedPlayer);
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
            key = legalMarkerKey;
            break;
        case Danger:
            key = dangerMarkerKey;
            break;
        case Motion:
            key = motionMarkerKey;
            break;
    }
    addMarker(pos, key);
}

void Board::addMarker(const Knights::Pos& pos, QString spriteKey)
{
    if (markers.contains(pos))
    {
        // Prevent two markers (usually Motion and Danger) from being on the same square
        delete markers[pos];
    }
    Item* marker = new Item ( renderer, spriteKey, this, pos);
    centerOnPos(marker, false);
    marker->setRenderSize ( QSizeF(m_tileSize, m_tileSize).toSize() );
    marker->setZValue ( legalMarkerZValue );
    markers.insert(pos, marker);
}

void Board::setPaused ( bool paused )
{
    m_paused = paused;
}


void Board::updateTheme()
{
    renderer->setTheme( Settings::theme() );
    #if not defined HAVE_RENDER
        // Using QGraphicsSvgItems, loading a new file and then resizing does not work
        // instead, we have to delete every item and re-create it
        foreach ( Piece* p, m_grid )
        {
            addPiece ( p->pieceType(), p->color(), m_grid.key ( p ) );
            delete p;
        }
        qDeleteAll(m_tiles);
        m_tiles.clear();
        addTiles();
        
        // If the user is changing the theme, he/she probably already saw any current markers
        qDeleteAll(markers);
        markers.clear();
    #endif
    updateGraphics();
}

void Board::updateGraphics()
{
    QSizeF tileSize = renderer->boundsOnSprite(whiteTileKey).size();
    QSizeF boardSize = 8 * tileSize;
    qreal sideMargin;
    qreal topMargin;
    if (renderer->spriteExists(lrBorderKey) && renderer->spriteExists(tbBorderKey))
    {
        sideMargin = renderer->boundsOnSprite(lrBorderKey).width();
        topMargin = renderer->boundsOnSprite(tbBorderKey).height();
    }
    else
    {
        sideMargin = 0.5 * tileSize.width();
        topMargin = 0.5 * tileSize.height();
        
        m_drawFrame = false;  
    }
    boardSize = boardSize + 2 * QSizeF(sideMargin, topMargin);
    qreal ratio = qMin(sceneRect().width()/boardSize.width(), sceneRect().height()/boardSize.height());
        
    QSizeF tpSize = tileSize * ratio;
    m_tileSize = floor ( qMin(tpSize.width(), tpSize.height()));
    sideMargin = qMax ( sideMargin * ratio, (sceneRect().width() - 8 * m_tileSize) / 2 );
    topMargin = qMax ( topMargin * ratio, (sceneRect().height() - 8 * m_tileSize) / 2 );
    m_boardRect = QRectF ( sideMargin, topMargin, m_tileSize * 8, m_tileSize * 8 );
    QSize tSize = QSizeF(m_tileSize, m_tileSize).toSize();
    
    foreach ( Piece* p, m_grid )
    {
        p->setRenderSize ( tSize );
        centerOnPos( p );
    }
    foreach ( Item* t, m_tiles )
    {
        t->setRenderSize ( tSize );
        centerOnPos( t, Settings::animateBoard() );
    }
    foreach ( Item* t, markers )
    {        
        t->setRenderSize ( tSize );
        centerOnPos( t );
    }
    emit centerChanged( QPointF( 4 * m_tileSize, 4 * m_tileSize ) );
}

void Board::changeDisplayedPlayer()
{
    foreach ( Piece* p, m_grid )
    {
        centerOnPos( p, m_grid.key( p ) );
    }
    foreach ( Item* i, markers )
    {
        centerOnPos( i, i->boardPos() );
    }
    if (Settings::animateBoard())
    {
        foreach ( Item* i, m_tiles )
        {
            centerOnPos( i, i->boardPos() );
        }
    }
}


#include "board.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on; 
