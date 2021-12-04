/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_SEEKGRAPH_H
#define KNIGHTS_SEEKGRAPH_H

#include <KPlotWidget>

#include <QMap>

namespace Knights {
struct FicsGameOffer;

class SeekGraph : public KPlotWidget {
	Q_OBJECT
public:
	explicit SeekGraph ( QWidget* parent = nullptr );
	~SeekGraph() override;

	void addSeek ( const FicsGameOffer& offer );
	void removeSeek ( int id );
	void clearOffers();
	void setRect ( const QRectF rect );

protected:
	void paintEvent ( QPaintEvent* event ) override;
	void mouseMoveEvent ( QMouseEvent* event ) override;
	void mouseReleaseEvent ( QMouseEvent* event ) override;

Q_SIGNALS:
	void seekClicked( int id );

private:
	QMap<KPlotPoint*, int> m_pointIds;
	QMap<int, KPlotObject*> m_objects;
	QRectF m_dataRect;
};

}

#endif // KNIGHTS_SEEKGRAPH_H
