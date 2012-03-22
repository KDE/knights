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

#include "proto/ficsdialog.h"
#include "ui_ficsdialog.h"
#include "settings.h"

#include <KDebug>
#include <KToolInvocation>
#include <KWallet/Wallet>

#include <QtGui/QCheckBox>
#include <QtGui/QTimeEdit>

using namespace Knights;
using KWallet::Wallet;

FicsDialog::FicsDialog ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f )
{
    ui = new Ui::FicsDialog;
    ui->setupUi ( this );

    for ( int i = 1; i < 4; ++i )
    {
        ui->tabWidget->setTabEnabled ( i, false );
    }
    ui->tabWidget->setCurrentIndex ( 0 );
    
    connect ( ui->tabWidget, SIGNAL (currentChanged(int)), SLOT (currentTabChanged(int)) );
    connect ( ui->seekButton, SIGNAL (toggled(bool)), SIGNAL (seekingChanged(bool)) );
    ui->seekButton->setIcon( KIcon ( QLatin1String("edit-find") ) );
    connect ( ui->logInButton, SIGNAL (clicked(bool)), SLOT (slotLogin()) );
    ui->logInButton->setIcon ( KIcon ( QLatin1String ( "network-connect" ) ) );

    connect ( ui->registerButton, SIGNAL (clicked(bool)), SLOT (slotCreateAccount()) );
    ui->registerButton->setIcon ( KIcon ( QLatin1String ( "list-add" ) ) );

    ui->usernameLineEdit->setText ( Settings::ficsUsername() );

    ui->challengeListView->setModel ( &m_challengeModel );

    connect ( ui->graphView, SIGNAL(seekClicked(int)), SIGNAL(acceptSeek(int)) );

    ui->rememberCheckBox->setChecked(Settings::autoLogin());

    connect ( ui->rememberCheckBox, SIGNAL(stateChanged(int)), this, SLOT(rememberCheckBoxChanged(int)) );
}

FicsDialog::~FicsDialog()
{
    delete ui;
}

QSize FicsDialog::sizeHint() const
{
    return QSize(800,500);
}

void FicsDialog::slotSessionStarted()
{
    setStatus ( i18n ( "Session started" ) );
    ui->logInButton->setEnabled ( false );
    for ( int i = 1; i < 4; ++i )
    {
        ui->tabWidget->setTabEnabled ( i, true );
    }
    ui->tabWidget->setCurrentIndex ( 1 );
    emit acceptButtonNeeded ( true );
    saveFicsSettings();
}

void FicsDialog::slotLogin()
{
    setLoginEnabled ( false );
    setStatus ( i18n("Logging in...") );
    emit login ( ui->usernameLineEdit->text(), ui->passwordLineEdit->text() );
}

void FicsDialog::slotCreateAccount()
{
    QUrl url;
    url.setScheme ( QLatin1String ( "http" ) );
    url.setHost ( serverName );
    if ( serverName == QLatin1String ( "freechess.org" ) )
    {
        url.setPath ( QLatin1String ( "/Register/index.html" ) );
    }
    KToolInvocation::invokeBrowser ( url.toString() );
}


void FicsDialog::addGameOffer ( const FicsGameOffer& offer )
{
    int row = ui->offerTable->rowCount();
    ui->offerTable->insertRow ( row );
    m_gameId << offer.gameId;
    
    ui->offerTable->setItem ( row, 0, new QTableWidgetItem ( offer.player.first ) );
    if ( offer.player.second != 0 )
    {
        ui->offerTable->setItem ( row, 1, new QTableWidgetItem ( QString::number ( offer.player.second ) ) );
    }

    QTimeEdit* baseTimeEdit = new QTimeEdit ( this );
    baseTimeEdit->setReadOnly ( true );
    baseTimeEdit->setDisplayFormat ( i18n ( "H:mm:ss" ) );
    baseTimeEdit->setButtonSymbols ( QAbstractSpinBox::NoButtons );
    QTime baseTime = QTime();
    baseTime.setHMS ( 0, offer.baseTime, 0 );
    baseTimeEdit->setTime ( baseTime );
    ui->offerTable->setCellWidget ( row, 2, baseTimeEdit );

    QTimeEdit* incTimeEdit = new QTimeEdit ( this );
    incTimeEdit->setReadOnly ( true );
    incTimeEdit->setDisplayFormat ( i18n ( "H:mm:ss" ) );
    incTimeEdit->setButtonSymbols ( QAbstractSpinBox::NoButtons );
    QTime incTime = QTime();
    incTime.setHMS ( 0, 0, offer.timeIncrement );
    incTimeEdit->setTime ( incTime );
    ui->offerTable->setCellWidget ( row, 3, incTimeEdit );

    QCheckBox* rated = new QCheckBox ( this );
    rated->setEnabled ( false );
    rated->setChecked ( offer.rated );
    ui->offerTable->setCellWidget ( row, 4, rated );
    ui->offerTable->setItem ( row, 5, new QTableWidgetItem ( offer.variant ) );

    ui->graphView->addSeek( offer );
}

void FicsDialog::addChallenge ( const FicsChallenge& challenge )
{
    QString item = i18nc ( "PlayerName (rating)", "%1 (%2)", challenge.player.first, challenge.player.second );
    m_challengeModel.setStringList ( m_challengeModel.stringList() << item );
    m_challengeId << challenge.gameId;
    emit declineButtonNeeded ( true );
}

void FicsDialog::clearOffers()
{
    ui->offerTable->setRowCount ( 0 );
    ui->graphView->clearOffers();
    m_gameId.clear();
}

void FicsDialog::accept()
{
    if ( ui->seekButton->isChecked() )
    {
        emit acceptChallenge( m_challengeId[ui->challengeListView->currentIndex().row()] );
    }
    else
    {
        emit acceptSeek ( m_gameId[ui->offerTable->currentRow() ] );
    }
}

void FicsDialog::decline()
{
    emit declineChallenge ( m_challengeId[ui->challengeListView->currentIndex().row()] );
}

void FicsDialog::currentTabChanged ( int tab )
{
    emit declineButtonNeeded ( tab == 3 );
    emit acceptButtonNeeded ( tab == 1 || tab == 2 || tab == 3 );
}

void FicsDialog::setServerName ( const QString& name )
{
    kDebug() << name;
    WId id = 0;
    if ( qApp->activeWindow() )
    {
        id = qApp->activeWindow()->winId();
    }
    QString password;
    Wallet* wallet = Wallet::openWallet ( Wallet::NetworkWallet(), id );
    if ( wallet )
    {
        QLatin1String folder ( "Knights" );
        if ( !wallet->hasFolder ( folder ) )
        {
            wallet->createFolder ( folder );
        }
        wallet->setFolder ( folder );
        QString key = ui->usernameLineEdit->text() + QLatin1Char ( '@' ) + name;
        wallet->readPassword ( key, password );
    }
    else
    {
        kDebug() << "KWallet not available";
    }
    ui->passwordLineEdit->setText ( password );
    serverName = name;
}

void FicsDialog::setConsoleWidget ( QWidget* widget )
{
    ui->tabWidget->widget ( 4 )->layout()->addWidget ( widget );
}

void FicsDialog::focusOnLogin()
{
    ui->tabWidget->setCurrentIndex ( 0 );
}

void FicsDialog::setStatus ( const QString& status, bool error )
{
    if ( error )
    {
        ui->logInStatusLabel->setText ( i18n ( "<font color='red'>Error: %1</font>", status ) );
    }
    else
    {
        ui->logInStatusLabel->setText ( status );
    }
}

bool FicsDialog::remember()
{
    return ui->rememberCheckBox->isChecked();
}

void FicsDialog::saveFicsSettings()
{
    Settings::setAutoLogin(ui->rememberCheckBox->isChecked());

    Settings::setFicsUsername ( ui->usernameLineEdit->text() );
    Settings::setGuest ( !ui->registeredCheckBox->isChecked() );

        WId id = 0;
        if ( qApp->activeWindow() )
        {
            id = qApp->activeWindow()->winId();
        }
        Wallet* wallet = Wallet::openWallet ( Wallet::NetworkWallet(), id );
        if ( wallet )
        {
            QLatin1String folder ( "Knights" );
            if ( !wallet->hasFolder ( folder ) )
            {
                wallet->createFolder ( folder );
            }
            wallet->setFolder ( folder );
            QString key = ui->usernameLineEdit->text() + QLatin1Char ( '@' ) + serverName;
            wallet->writePassword ( key, ui->passwordLineEdit->text() );
        }
        Settings::self()->writeConfig();
}

void FicsDialog::removeGameOffer ( int id )
{
    if (!m_gameId.contains(id))
    {
        return;
    }
    ui->offerTable->removeRow(m_gameId.indexOf(id));
    ui->graphView->removeSeek(id);
    m_gameId.removeAll(id);
}

void FicsDialog::setLoginEnabled ( bool enable )
{
    ui->logInButton->setEnabled ( enable );
}

void FicsDialog::removeChallenge ( int id )
{
    m_challengeModel.removeRows ( m_challengeId.indexOf(id), 1 );
    m_challengeId.removeAll(id);
}

bool FicsDialog::autoAcceptChallenge()
{
    return ui->autoCheckBox->isChecked();
}

bool FicsDialog::rated()
{
    return ui->ratedCheckBox->isChecked();
}

void FicsDialog::rememberCheckBoxChanged( int state )
{
    Q_UNUSED(state)
    Settings::setAutoLogin(ui->rememberCheckBox->isChecked());
    Settings::self()->writeConfig();
}

// kate: indent-mode cstyle; space-indent on; indent-width 4; replace-tabs on;  replace-tabs on;  replace-tabs on;  replace-tabs on;
