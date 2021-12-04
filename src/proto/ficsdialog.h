/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_FICSDIALOG_H
#define KNIGHTS_FICSDIALOG_H

#include "proto/ficsprotocol.h"

#include <QWidget>
#include <QStringListModel>

namespace Ui {
class FicsDialog;
}

namespace Knights {
class FicsDialog : public QWidget {
	Q_OBJECT
public:
    explicit FicsDialog ( QWidget* parent = nullptr, Qt::WindowFlags f = {} );
	~FicsDialog() override;

	QString userName();
	QString password();
	bool remember();

	void setServerName(const QString& name);

	QSize sizeHint() const override;

	bool autoAcceptChallenge();
	bool rated();

public Q_SLOTS:
	void slotSessionStarted();
	void addGameOffer ( const FicsGameOffer& offer );
	void removeGameOffer ( int id );
	void addChallenge ( const FicsChallenge& challenge );
	void removeChallenge ( int id );

	void clearOffers();
	void accept();
	void decline();

	void currentTabChanged ( int tab );
	void slotCreateAccount();
	void slotLogin();
	void setConsoleWidget(QWidget* widget);
	void focusOnLogin();
	void setStatus(const QString& status, bool error = false);
	void setLoginEnabled ( bool enable );

Q_SIGNALS:
    void login ( const QString &username, const QString &password );

	void seekingChanged ( bool seek );
	void declineButtonNeeded ( bool needed );
	void acceptButtonNeeded ( bool needed );

	void acceptSeek ( int seekId );
	void acceptChallenge ( int challengeId );
	void declineChallenge ( int challengeId );

private Q_SLOTS:
	void rememberCheckBoxChanged( int state );

private:
	void saveFicsSettings();
	Ui::FicsDialog* ui;
	QList<int> m_gameId;
	QList<int> m_challengeId;
	QStringListModel m_challengeModel;
	QString serverName;
};
}

#endif // KNIGHTS_FICSDIALOG_H
