/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2011 Miha Čančula <miha@noughmad.eu>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef KNIGHTS_COMPUTERPROTOCOL_H
#define KNIGHTS_COMPUTERPROTOCOL_H

#include <proto/textprotocol.h>

class KProcess;

namespace Knights {

class ComputerProtocol : public TextProtocol {
public:
	explicit ComputerProtocol(QObject* parent = nullptr);
	~ComputerProtocol() override;

	virtual void startProgram();
	bool isComputer() override;
	QList< ToolWidgetData > toolWidgets() override;

protected:
	KProcess* mProcess;

private Q_SLOTS:
	void readError();
};

}

#endif // KNIGHTS_COMPUTERPROTOCOL_H
