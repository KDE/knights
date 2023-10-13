/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_ITEM_H
#define KNIGHTS_ITEM_H

#include "pos.h"

#include <KGameRenderedGraphicsObject>

namespace Knights {

class Item : public KGameRenderedGraphicsObject {
	Q_OBJECT
	Q_PROPERTY ( Pos boardPos READ boardPos WRITE setBoardPos )
	Q_PROPERTY ( QSize renderSize READ renderSize WRITE setRenderSize )

public:
	Item ( KGameGraphicsViewRenderer* renderer, const QString& key, QGraphicsScene* scene, Pos boardPos, QGraphicsItem* parentItem = nullptr );
	~Item() override;

	void setBoardPos ( const Pos& pos );
	Pos boardPos() const;

	void move ( const QPointF& pos, qreal tileSize, bool animated = true );
	void resize ( const QSize& size, bool animated = true );
	void moveAndResize ( const QPointF& pos, qreal tileSize, const QSize& size, bool animated = true );

private:
	Pos m_pos;
};
}
#endif // KNIGHTS_ITEM_H
