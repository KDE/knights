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

#include "board.h"
#include "settings.h"

#include "core/pos.h"
#include "core/move.h"
#include "core/item.h"
#include "rules/rules.h"

#include "ui_promotiondialog.h"
#include "gamemanager.h"

#include <KgTheme>
#include <KGameRenderer>
#include <KDebug>
#include <KDialog>

#include <QtCore/QMap>
#include <QtGui/QDropEvent>
#include <QtSvg/QGraphicsSvgItem>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsView>
#include <QtCore/qmath.h>


using namespace Knights;

const qreal backgroundZValue = -3.0;
const qreal borderZValue = -2.0;
const qreal notationZValue = -1.0;
const qreal tileZValue = 0.0;
const qreal pieceZValue = 1.0;
const qreal motionMarkerZValue = 2.0;
const qreal legalMarkerZValue = 3.0;
const qreal dragZValue = 4.0;

const QString backgroundKey = QLatin1String ( "Background" );
const QString whiteTileKey = QLatin1String ( "WhiteTile" );
const QString blackTileKey = QLatin1String ( "BlackTile" );
const QString legalMarkerKey = QLatin1String ( "Marker" );
const QString motionMarkerKey = QLatin1String ( "Motion" );
const QString dangerMarkerKey = QLatin1String ( "Danger" );

const QString tbBorderKey = QLatin1String ( "TopBottomBorder" );
const QString lrBorderKey = QLatin1String ( "LeftRightBorder" );
const QString whiteLettersKey = QLatin1String ( "WhiteLetters" );
const QString blackLettersKey = QLatin1String ( "BlackLetters" );
const QString whiteNumbersKey = QLatin1String ( "WhiteNumbers" );
const QString blackNumbersKey = QLatin1String ( "BlackNumbers" );

Board::Board ( KgThemeProvider* provider, QObject* parent ) : QGraphicsScene ( parent ), 
m_themeProvider(provider)
{
    renderer = new KGameRenderer ( m_themeProvider );
    m_background = 0;
    selectedPiece = 0;
    draggedPiece = 0;
    Manager::self()->rules()->setGrid ( &m_grid );
    m_currentPlayer = White;
    updateTheme();
    m_dragActive = false;
    
    connect (provider, SIGNAL(currentThemeChanged(const KgTheme*)), SLOT(updateTheme()));
}

Board::~Board()
{
    qDeleteAll ( m_grid );
    qDeleteAll ( m_tiles );
    qDeleteAll ( markers );
    delete renderer;
}

void Board::addPiece ( PieceType type, Color color, const Pos& pos )
{
    Piece* t_piece = new Piece ( renderer, type, color, this, pos );
    if ( Settings::animationSpeed() != Settings::EnumAnimationSpeed::Instant )
    {
        t_piece->setPos ( mapToScene ( Pos ( ( pos.first > 4 ) ? 5 : 4, ( pos.second > 4 ) ? 5 : 4 ) ) );
    }
    t_piece->setZValue ( pieceZValue );
    m_grid.insert ( pos, t_piece );
}

void Board::movePiece ( const Move& move )
{
    kDebug() << move;
    Move m = move;
    if ( ( m.flag ( Move::Illegal ) && !m.flag ( Move::Forced ) ) ||  m.to() == m.from() || !m_grid.contains ( m.from() ) )
    {
        kWarning() << "Invalid move:" << m;
        return;
    }
    if ( !m.flag ( Move::Forced ) &&
        ( m_grid[m.from()]->color() != m_currentPlayer || !Manager::self()->rules()->legalMoves(m.from()).contains(m) ) )
    {
        kWarning() << "Move not allowed:" << m;
        return;
    }
    qDeleteAll ( markers );
    markers.clear();
    if ( m.flag(Move::Promote) )
    {
        m_grid[m.from() ]->setPieceType ( m.promotedType() ? m.promotedType() : Queen );
    }

    PieceDataMap map = m.removedPieces();
    PieceDataMap::const_iterator it = map.constBegin();
    PieceDataMap::const_iterator end = map.constEnd();
    for ( ; it != end; ++it )
    {
        delete m_grid.value ( it.key(), 0 );
        m_grid.remove ( it.key() );
    }

    centerOnPos ( m_grid.value ( m.from() ), m.to() );
    m_grid.insert ( m.to(), m_grid.take ( m.from() ) );

    map = m.addedPieces();
    it = map.constBegin();
    end = map.constEnd();
    for ( ; it != end; ++it )
    {
        addPiece ( it.value().second, it.value().first, it.key() );
    }

    if ( m_playerColors & oppositeColor ( m_currentPlayer ) )
    {
        // We only display motion and danger markers if the next player is a human
        if ( Settings::showMotion() )
        {
            addMarker ( m.from(), Motion );
            addMarker ( m.to(), Motion );
        }
        if ( Settings::showDanger() )
        {
            bool check = false;
            foreach ( Piece* piece, m_grid )
            {
                if ( piece->color() == m_currentPlayer && Manager::self()->rules()->isAttacking ( piece->boardPos() ) )
                {
                    check = true;
                    addMarker ( piece->boardPos(), Danger );
                }
            }
            if ( check )
            {
                foreach ( Piece* piece, m_grid )
                {
                    if ( piece->color() != m_currentPlayer && piece->pieceType() == King )
                    {
                        addMarker ( piece->boardPos(), Danger );
                    }
                }
            }
        }
    }

    foreach ( const Move& additionalMove, m.additionalMoves() )
    {
        movePiece ( additionalMove );
    }

    updateGraphics();
}

void Board::populate()
{
    const PieceDataMap pieces = Manager::self()->rules()->startingPieces();
    PieceDataMap::const_iterator it = pieces.constBegin();
    PieceDataMap::const_iterator end = pieces.constEnd();
    for ( ; it != end; ++it )
    {
        addPiece ( it.value().second, it.value().first, it.key() );
    }
    updateGraphics();
}

void Board::addTiles()
{
    if ( !m_tiles.isEmpty() )
    {
        kWarning() << "Tiles are already present, delete them first";
        return;
    }
    for ( int i = 1; i < 9; ++i )
    {
        for ( int j = 1; j < 9; ++j )
        {
            QString key = ( ( i + j ) % 2 == 0 ) ? blackTileKey : whiteTileKey;
            Item* tile = new Item ( renderer, key, this, Pos ( i, j ) );
            tile->setZValue ( tileZValue );
            m_tiles.insert ( Pos ( i, j ), tile );
        }
    }
}

void Board::mousePressEvent ( QGraphicsSceneMouseEvent* e )
{
    if ( !Manager::self()->canLocalMove() )
    {
        // It is not the human player's turn
        e->ignore();
        return;
    }
    
    Piece* d_piece = pieceAt ( e->scenePos() );
    if ( !d_piece || d_piece->color() != m_currentPlayer )
    {
        // The piece doesn't belong to the player whose turn it is, or there is no piece
        if ( !selectedPiece )
        {
            e->ignore();
            return;
        }
        Pos from = selectedPiece->boardPos();
        Pos to = mapFromScene ( e->scenePos() );
        if ( Manager::self()->rules()->legalMoves ( from ).contains ( Move ( from, to ) ) )
        {
            Move move ( from, to );
            move.setFlag ( Move::Take, m_grid.contains ( to ) );
            
            if ( m_grid[from]->pieceType() == Pawn && ( to.second == 1 || to.second == 8 ) )
            {
                move.setFlag ( Move::Promote, true );
                move.setPromotedType ( getPromotedType() );
            }
            emit pieceMoved(move);
            selectedPiece = 0;
        }
    }
    else
    {
        // The active player clicked on his/her own piece
        qDeleteAll ( markers );
        markers.clear();
        
        selectedPiece = d_piece;
        
        Pos t_pos = mapFromScene ( e->scenePos() );
        QList<Move> t_legalMoves = Manager::self()->rules()->legalMoves ( t_pos );
        if ( t_legalMoves.isEmpty() )
        {
            e->ignore();
            return;
        }
        d_piece->setZValue ( dragZValue );
        if ( Settings::showMarker() )
        {
            foreach ( const Move& t_move, t_legalMoves )
            {
                addMarker ( t_move.to(), LegalMove );
            }
        }
        draggedPiece = d_piece;
        m_draggedPos = e->scenePos();
        dragStartPoint = e->screenPos();
    }
}

void Board::mouseReleaseEvent(QGraphicsSceneMouseEvent* e)
{
    Q_UNUSED(e);
    draggedPiece = 0;
}


void Board::mouseMoveEvent ( QGraphicsSceneMouseEvent* e )
{
    if (!(e->buttons() & Qt::LeftButton) || m_dragActive)
    {
        return;
    }
    if (draggedPiece && ((e->screenPos() - dragStartPoint).manhattanLength() >= QApplication::startDragDistance()) )
    {
        //initiate a new drag event
        drag = new QDrag ( e->widget() );
        drag->setMimeData ( new QMimeData() );
        m_dragActive = true;
        selectedPiece = 0;
        drag->exec();
    }
}

void Board::dragLeaveEvent(QGraphicsSceneDragDropEvent* e)
{
    Q_UNUSED(e);
    if ( !m_dragActive )
    {
        return;
    }
    qDeleteAll ( markers );
    markers.clear();
    centerOnPos ( draggedPiece );
    draggedPiece->setZValue ( pieceZValue );
    draggedPiece = 0;
    m_dragActive = false;
}

void Board::dropEvent ( QGraphicsSceneDragDropEvent* e )
{
    qDeleteAll ( markers );
    markers.clear();

    if ( draggedPiece )
    {
        m_dragActive = false;
        Pos from = draggedPiece->boardPos();
        Pos to = mapFromScene ( e->scenePos() );
        Move move ( from, to );
        if ( !Manager::self()->rules()->legalMoves ( from ).contains ( move ) )
        {
            centerOnPos ( draggedPiece );
        }
        else
        {
            if ( m_grid[from]->pieceType() == Pawn && ( to.second == 1 || to.second == 8 ) )
            {
                move.setFlag ( Move::Promote, true );
                move.setPromotedType ( getPromotedType() );
            }
            emit pieceMoved(move);
        }
        draggedPiece->setZValue ( pieceZValue );
        draggedPiece = 0;
    }
}

void Board::dragEnterEvent ( QGraphicsSceneDragDropEvent* e )
{
    e->setAccepted ( Manager::self()->canLocalMove() );
}

void Board::dragMoveEvent ( QGraphicsSceneDragDropEvent* e )
{
    if ( !draggedPiece )
    {
        e->ignore();
        return;
    }
    e->accept();
    qreal x = e->scenePos().x() - m_draggedPos.x();
    qreal y = e->scenePos().y() - m_draggedPos.y();

    draggedPiece->moveBy ( x, y );
    m_draggedPos = e->scenePos();
}

Piece* Board::pieceAt ( const QPointF& point )
{
    return m_grid.value ( mapFromScene ( point ), 0 );
}

Pos Board::mapFromScene ( const QPointF& point )
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

void Board::centerOnPos ( Item* item, const Pos& pos, bool animated )
{
    item->setBoardPos ( pos );
    centerOnPos ( item, animated );
}

void Board::centerOnPos ( Item* item, bool animated )
{
    item->move ( mapToScene ( item->boardPos() ), m_tileSize, animated );
}

void Board::centerAndResize ( Item* item, QSize size, bool animated )
{
    item->moveAndResize ( mapToScene ( item->boardPos() ), m_tileSize, size, animated );
}

bool Board::isInBoard ( const Pos& pos )
{
    return pos.first > 0 && pos.first < 9 && pos.second > 0 && pos.second < 9;
}

Colors Board::playerColors() const
{
    return m_playerColors;
}

void Board::setPlayerColors ( Colors colors )
{
    m_playerColors = colors;
    if ( m_playerColors & m_currentPlayer )
    {
        m_displayedPlayer = m_currentPlayer;
    }
    else
    {
        if ( m_playerColors == Black )
        {
            m_displayedPlayer = Black;
        }
        else
        {
            m_displayedPlayer = White;
        }
    }
    changeDisplayedPlayer();
    populate();
}

void Board::setCurrentColor ( Color color )
{
    m_currentPlayer = color;
    Color nextPlayer = m_displayedPlayer;
    if ( ( ( m_playerColors & (Black|White) ) == (Black|White) ) && !Settings::flipBoard() )
    {
        nextPlayer = White;
    }
    else if ( m_playerColors & color )
    {
        nextPlayer = color;
    }
    if ( m_displayedPlayer != nextPlayer )
    {
        m_displayedPlayer = nextPlayer;
        changeDisplayedPlayer();
    }
    emit activePlayerChanged ( m_currentPlayer );
}


void Board::addMarker ( const Pos& pos, MarkerType type )
{
    QString key;
    switch ( type )
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
    addMarker ( pos, key );
}

void Board::addMarker ( const Pos& pos, const QString& spriteKey )
{
    if ( markers.contains ( pos ) )
    {
        // Prevent two markers (usually Motion and Danger) from being on the same square
        delete markers[pos];
    }
    Item* marker = new Item ( renderer, spriteKey, this, pos );
    centerOnPos ( marker, false );
    marker->setRenderSize ( QSizeF ( m_tileSize, m_tileSize ).toSize() );
    marker->setZValue ( legalMarkerZValue );
    markers.insert ( pos, marker );
}

void Board::updateTheme()
{
    delete m_background;
    if ( renderer->spriteExists ( backgroundKey ) )
    {
        m_background = new Item ( renderer, backgroundKey, this, Pos() );
        m_background->setZValue ( backgroundZValue );
    }
    
    qDeleteAll ( m_borders );
    m_borders.clear();
    qDeleteAll ( m_notations );
    m_notations.clear();
    m_displayBorders = Settings::borderDisplayType() != Settings::EnumBorderDisplayType::None
                       && renderer->spriteExists ( lrBorderKey )
                       && renderer->spriteExists ( tbBorderKey );
    m_displayNotations = Settings::borderDisplayType() == Settings::EnumBorderDisplayType::Notation
                         && renderer->spriteExists ( whiteLettersKey )
                         && renderer->spriteExists ( blackLettersKey )
                         && renderer->spriteExists ( whiteNumbersKey )
                         && renderer->spriteExists ( blackNumbersKey );
    if ( m_displayBorders )
    {
        m_borders << new Item ( renderer, tbBorderKey, this, Pos() );

        Item *tItem = new Item ( renderer, lrBorderKey, this, Pos() );
        tItem->setRotation ( 180 );
        m_borders << tItem;

        tItem = new Item ( renderer, tbBorderKey, this, Pos() );
        tItem->setRotation ( 180 );
        m_borders << tItem;

        m_borders << new Item ( renderer, lrBorderKey, this, Pos() );
        foreach ( Item* item, m_borders )
        {
            item->setZValue ( borderZValue );
        }
    }
    if ( m_displayNotations )
    {
        QString lettersKey;
        QString numbersKey;
        if ( m_displayedPlayer == White )
        {
            lettersKey = whiteLettersKey;
            numbersKey = whiteNumbersKey;
        }
        else
        {
            lettersKey = blackLettersKey;
            numbersKey = blackNumbersKey;
        }
        m_notations << new Item ( renderer, lettersKey, this, Pos() );
        m_notations << new Item ( renderer, numbersKey, this, Pos() );
        m_notations << new Item ( renderer, lettersKey, this, Pos() );
        m_notations << new Item ( renderer, numbersKey, this, Pos() );
        foreach ( Item* item, m_notations )
        {
            item->setZValue ( notationZValue );
        }
    }
    addTiles();
    updateGraphics();
}

void Board::updateGraphics()
{
    if ( m_background )
    {
        m_background->setRenderSize ( sceneRect().size().toSize() );
    }
    QSizeF tileSize = renderer->boundsOnSprite ( whiteTileKey ).size();
    QSizeF boardSize = 8 * tileSize;
    qreal sideMargin;
    qreal topMargin;
    if ( m_displayBorders )
    {
        sideMargin = renderer->boundsOnSprite ( lrBorderKey ).width();
        topMargin = renderer->boundsOnSprite ( tbBorderKey ).height();
    }
    else
    {
        sideMargin = 0.0;
        topMargin = 0.0;
    }
    boardSize = boardSize + 2 * QSize ( sideMargin, topMargin );
    qreal ratio = qMin ( sceneRect().width() / boardSize.width(), sceneRect().height() / boardSize.height() );
    sideMargin *= ratio;
    topMargin *= ratio;

    QSizeF tpSize = tileSize * ratio;
    m_tileSize = qFloor ( qMin ( tpSize.width(), tpSize.height() ) );
    /*
    if ( m_displayBorders )
    {
        if (  m_tileSize % 2 )
        {
            m_tileSize -= 1;
        }
        sideMargin = m_tileSize / 2;
        topMargin = m_tileSize / 2;
    }
    */
    
    QSize hBorderSize = QSize ( 8 * m_tileSize + 2 * qRound ( sideMargin ), qRound ( topMargin ) );
    QSize vBorderSize = QSize ( qRound ( sideMargin ), 8 * m_tileSize );
    int hBorderMargin = qRound ( topMargin );
    int vBorderMargin = qRound ( sideMargin );
    

    sideMargin = qMax ( sideMargin, ( sceneRect().width() - 8 * m_tileSize ) / 2 );
    topMargin = qMax ( topMargin, ( sceneRect().height() - 8 * m_tileSize ) / 2 );
    m_boardRect = QRect( sceneRect().topLeft().toPoint() + QPoint( sideMargin, topMargin ),
                          QSize( m_tileSize, m_tileSize ) * 8);
    
    QSize tSize = QSizeF ( m_tileSize, m_tileSize ).toSize();
    /*
     * For historical reasons, QRect's 
     */
    QPointF topBorderPoint = m_boardRect.topRight() + QPoint ( vBorderMargin, 0 );
    QPointF rightBorderPoint = m_boardRect.bottomRight() + QPoint ( vBorderMargin, 0 );
    QPointF bottomBorderPoint = m_boardRect.bottomLeft() - QPoint ( vBorderMargin, 0 );
    QPointF leftBorderPoint = m_boardRect.topLeft() - QPoint ( vBorderMargin, 0 );
    
    foreach ( Piece* p, m_grid )
    {
        centerAndResize ( p, tSize );
    }
    foreach ( Item* t, m_tiles )
    {
        centerAndResize ( t, tSize, Settings::animateBoard() );
    }
    foreach ( Item* t, markers )
    {
        centerAndResize ( t, tSize );
    }
    if ( m_displayBorders )
    {
        m_borders[0]->moveAndResize ( bottomBorderPoint, m_tileSize, hBorderSize, Settings::animateBoard() );
        m_borders[1]->moveAndResize ( rightBorderPoint, m_tileSize, vBorderSize, Settings::animateBoard() );
        m_borders[2]->moveAndResize ( topBorderPoint, m_tileSize, hBorderSize, Settings::animateBoard() );
        m_borders[3]->moveAndResize ( leftBorderPoint, m_tileSize, vBorderSize, Settings::animateBoard() );
    }
    if ( m_displayNotations )
    {
        m_notations[0]->moveAndResize ( bottomBorderPoint, m_tileSize, hBorderSize, Settings::animateBoard() );
        m_notations[1]->moveAndResize ( m_boardRect.topRight(), m_tileSize, vBorderSize, Settings::animateBoard() );
        m_notations[2]->moveAndResize ( m_boardRect.topLeft() - QPointF ( vBorderMargin, hBorderMargin ), m_tileSize, hBorderSize, Settings::animateBoard() );
        m_notations[3]->moveAndResize ( leftBorderPoint, m_tileSize, vBorderSize, Settings::animateBoard() );
    }
}

void Board::changeDisplayedPlayer()
{
    foreach ( Piece* p, m_grid )
    {
        centerOnPos ( p );
    }
    foreach ( Item* i, markers )
    {
        centerOnPos ( i );
    }
    if ( Settings::animateBoard() )
    {
        foreach ( Item* i, m_tiles )
        {
            centerOnPos ( i );
        }
    }
    if ( m_displayNotations )
    {
        if ( m_displayedPlayer == White )
        {
            m_notations[0]->setSpriteKey ( whiteLettersKey );
            m_notations[1]->setSpriteKey ( whiteNumbersKey );
            m_notations[2]->setSpriteKey ( whiteLettersKey );
            m_notations[3]->setSpriteKey ( whiteNumbersKey );
        }
        else
        {
            m_notations[0]->setSpriteKey ( blackLettersKey );
            m_notations[1]->setSpriteKey ( blackNumbersKey );
            m_notations[2]->setSpriteKey ( blackLettersKey );
            m_notations[3]->setSpriteKey ( blackNumbersKey );
        }
    }
    emit displayedPlayerChanged ( m_displayedPlayer );
}

PieceType Board::getPromotedType()
{
    KDialog dialog;
    dialog.setButtons ( KDialog::Ok );
    dialog.setButtonText ( KDialog::Ok, i18n ( "Promote" ) );
    dialog.setCaption ( i18n ( "Promotion" ) );
    QWidget promotionWidget ( &dialog );
    Ui::PromotionWidget ui;
    ui.setupUi ( &promotionWidget );
    dialog.setMainWidget ( &promotionWidget );
    if ( dialog.exec() == KDialog::Accepted )
    {
        if ( ui.radioButtonQueen->isChecked() )
        {
            return Queen;
        }
        else if ( ui.radioButtonKnight->isChecked() )
        {
            return Knight;
        }
        else if ( ui.radioButtonBishop->isChecked() )
        {
            return Bishop;
        }
        else if ( ui.radioButtonRook->isChecked() )
        {
            return Rook;
        }
    }
    return Queen;
}

#include "board.moc"
// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
