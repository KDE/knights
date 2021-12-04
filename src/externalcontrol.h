/*
 *  This file is part of Knights, a chess board for KDE SC 4.
 *  SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>
 *
 *  SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
 */

#ifndef KNIGHTS_EXTERNALCONTROL_H
#define KNIGHTS_EXTERNALCONTROL_H

#include <QObject>

namespace Knights {

class Move;

class ExternalControl : public QObject {
	Q_OBJECT
	Q_CLASSINFO("D-Bus Interface", "org.kde.Knights")

public:
	explicit ExternalControl(QObject* parent = nullptr);
	~ExternalControl() override;

public Q_SLOTS:
	void movePiece(const QString& move);
	void pauseGame();
	void resumeGame();
	void undo();
	void offerDraw();
	void adjourn();
	void abort();
	void slotMoveMade(const Move& move);

Q_SIGNALS:
	void moveMade(const QString& move);
};

}

#endif // KNIGHTS_EXTERNALCONTROL_H
