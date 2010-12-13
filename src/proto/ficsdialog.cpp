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

#include "ficsdialog.h"

#include "ui_ficsdialog.h"

#include <KDebug>

#include <QtGui/QCheckBox>
#include <QtGui/QTimeEdit>

using namespace Knights;

FicsDialog::FicsDialog ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::FicsDialog;
    ui->setupUi ( this );

    connect ( ui->tabWidget, SIGNAL ( currentChanged ( int ) ), SLOT ( currentTabChanged ( int ) ) );
    connect ( ui->refreshButton, SIGNAL ( clicked ( bool ) ), SLOT ( refresh() ) );
    connect ( ui->seekButton, SIGNAL ( toggled ( bool ) ), SIGNAL ( seekingChanged ( bool ) ) );
}

FicsDialog::~FicsDialog()
{
    delete ui;
}

void FicsDialog::addGameOffer ( const Knights::FicsGameOffer& offer )
{
    int row = ui->offerTable->rowCount();
    ui->offerTable->insertRow ( row );

    m_gameId[row] = offer.gameId;

    ui->offerTable->setItem ( row, 0, new QTableWidgetItem ( offer.player.first ) );
    if ( offer.player.second != 0 )
    {
        ui->offerTable->setItem ( row, 1, new QTableWidgetItem ( QString::number ( offer.player.second ) ) );
    }

    QTimeEdit* baseTimeEdit = new QTimeEdit ( this );
    baseTimeEdit->setReadOnly ( true );
    baseTimeEdit->setDisplayFormat ( i18n ( "H:mm:ss" ) );
    baseTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QTime baseTime = QTime();
    baseTime.setHMS ( 0, offer.baseTime, 0 );
    baseTimeEdit->setTime ( baseTime );
    ui->offerTable->setCellWidget ( row, 2, baseTimeEdit );

    QTimeEdit* incTimeEdit = new QTimeEdit ( this );
    incTimeEdit->setReadOnly ( true );
    incTimeEdit->setDisplayFormat ( i18n ( "H:mm:ss" ) );
    incTimeEdit->setButtonSymbols(QAbstractSpinBox::NoButtons);
    QTime incTime = QTime();
    incTime.setHMS ( 0, 0, offer.timeIncrement );
    incTimeEdit->setTime ( incTime );
    ui->offerTable->setCellWidget ( row, 3, incTimeEdit );

    QCheckBox* rated = new QCheckBox ( this );
    rated->setEnabled ( false );
    rated->setChecked ( offer.rated );
    ui->offerTable->setCellWidget ( row, 4, rated );
    ui->offerTable->setItem ( row, 5, new QTableWidgetItem ( offer.variant ) );
}

void FicsDialog::addChallenge ( const Knights::FicsPlayer& challenger )
{
    ui->challengeLabel->setText ( i18n ( "Challenge from %1 (%2)", challenger.first, challenger.second ) );
}

void FicsDialog::clearOffers()
{
    ui->offerTable->setRowCount ( 0 );
    m_gameId.clear();
}

void FicsDialog::refresh()
{
    clearOffers();
    emit sought();
}

void FicsDialog::accept()
{
    if ( ui->seekButton->isChecked() )
    {
        emit acceptChallenge();
    }
    else
    {
        emit acceptSeek ( m_gameId[ui->offerTable->currentRow() ] );
    }
}

void FicsDialog::decline()
{
    emit declineChallenge();
}

void FicsDialog::currentTabChanged ( int tab )
{
    emit declineButtonNeeded ( tab == 1 );
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;
