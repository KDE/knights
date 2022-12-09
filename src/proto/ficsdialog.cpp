/*
    This file is part of Knights, a chess board for KDE SC 4.
    SPDX-FileCopyrightText: 2009, 2010, 2011 Miha Čančula <miha@noughmad.eu>
    SPDX-FileCopyrightText: 2016 Alexander Semke <alexander.semke@web.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "proto/ficsdialog.h"
#include "ui_ficsdialog.h"
#include "settings.h"
#include "knightsdebug.h"

#include <KWallet>

#include <QDesktopServices>

using namespace Knights;
using KWallet::Wallet;

FicsDialog::FicsDialog ( QWidget* parent, Qt::WindowFlags f ) : QWidget ( parent, f ) {
	ui = new Ui::FicsDialog;
	ui->setupUi ( this );

	for ( int i = 1; i < 4; ++i )
		ui->tabWidget->setTabEnabled ( i, false );
	ui->tabWidget->setCurrentIndex ( 0 );

	connect ( ui->tabWidget, &QTabWidget::currentChanged, this, &FicsDialog::currentTabChanged );
	connect ( ui->seekButton, &QPushButton::toggled, this, &FicsDialog::seekingChanged );
	ui->seekButton->setIcon( QIcon::fromTheme ( QStringLiteral("edit-find") ) );
	connect ( ui->logInButton, &QPushButton::clicked, this, &FicsDialog::slotLogin );
	ui->logInButton->setIcon ( QIcon::fromTheme ( QStringLiteral ( "network-connect" ) ) );

	connect ( ui->registerButton, &QPushButton::clicked, this, &FicsDialog::slotCreateAccount );
	ui->registerButton->setIcon ( QIcon::fromTheme ( QStringLiteral ( "list-add" ) ) );

	ui->usernameLineEdit->setText ( Settings::ficsUsername() );

	ui->challengeListView->setModel ( &m_challengeModel );

	connect ( ui->graphView, &SeekGraph::seekClicked, this, &FicsDialog::acceptSeek );

	ui->rememberCheckBox->setChecked(Settings::autoLogin());

	connect ( ui->rememberCheckBox, &QCheckBox::stateChanged, this, &FicsDialog::rememberCheckBoxChanged );
}

FicsDialog::~FicsDialog() {
	delete ui;
}

QSize FicsDialog::sizeHint() const {
	return QSize(800,500);
}

void FicsDialog::slotSessionStarted() {
	setStatus ( i18n ( "Session started" ) );
	ui->logInButton->setEnabled ( false );
	for ( int i = 1; i < 4; ++i )
		ui->tabWidget->setTabEnabled ( i, true );
	ui->tabWidget->setCurrentIndex ( 1 );
	Q_EMIT acceptButtonNeeded ( true );
	saveFicsSettings();
}

void FicsDialog::slotLogin() {
	setLoginEnabled ( false );
	setStatus ( i18n("Logging in...") );
	Q_EMIT login ( ui->usernameLineEdit->text(), ui->passwordLineEdit->text() );
}

void FicsDialog::slotCreateAccount() {
	QUrl url;
	url.setScheme ( QStringLiteral ( "https" ) );
	url.setHost ( serverName );
	if ( serverName == QLatin1String ( "freechess.org" ) )
		url.setPath ( QStringLiteral ( "/Register/index.html" ) );
	QDesktopServices::openUrl(url);
}


void FicsDialog::addGameOffer ( const FicsGameOffer& offer ) {
	int row = ui->offerTable->rowCount();
	ui->offerTable->insertRow ( row );
	m_gameId << offer.gameId;

	ui->offerTable->setItem ( row, 0, new QTableWidgetItem ( offer.player.first ) );
	if ( offer.player.second != 0 )
		ui->offerTable->setItem ( row, 1, new QTableWidgetItem ( QString::number ( offer.player.second ) ) );

	QTime baseTime = QTime();
	baseTime.setHMS ( 0, offer.baseTime, 0 );
	ui->offerTable->setItem ( row, 2, new QTableWidgetItem (baseTime.toString()) );

	QTime incTime = QTime();
	incTime.setHMS ( 0, 0, offer.timeIncrement );
	ui->offerTable->setItem ( row, 3, new QTableWidgetItem ( incTime.toString()) );

	QCheckBox* rated = new QCheckBox ( this );
	rated->setEnabled ( false );
	rated->setChecked ( offer.rated );
	ui->offerTable->setCellWidget ( row, 4, rated );
	ui->offerTable->setItem ( row, 5, new QTableWidgetItem ( offer.variant ) );
	ui->offerTable->resizeColumnToContents(0);
	ui->graphView->addSeek( offer );
}

void FicsDialog::addChallenge ( const FicsChallenge& challenge ) {
	QString item = i18nc ( "PlayerName (rating)", "%1 (%2)", challenge.player.first, challenge.player.second );
	m_challengeModel.setStringList ( m_challengeModel.stringList() << item );
	m_challengeId << challenge.gameId;
	Q_EMIT declineButtonNeeded ( true );
}

void FicsDialog::clearOffers() {
	ui->offerTable->setRowCount ( 0 );
	ui->graphView->clearOffers();
	m_gameId.clear();
}

void FicsDialog::accept() {
	if ( ui->seekButton->isChecked() )
		Q_EMIT acceptChallenge( m_challengeId[ui->challengeListView->currentIndex().row()] );
	else
		Q_EMIT acceptSeek ( m_gameId[ui->offerTable->currentRow() ] );
}

void FicsDialog::decline() {
	Q_EMIT declineChallenge ( m_challengeId[ui->challengeListView->currentIndex().row()] );
}

void FicsDialog::currentTabChanged ( int tab ) {
	Q_EMIT declineButtonNeeded ( tab == 3 );
	Q_EMIT acceptButtonNeeded ( tab == 1 || tab == 2 || tab == 3 );
}

void FicsDialog::setServerName ( const QString& name ) {
	qCDebug(LOG_KNIGHTS) << name;
	WId id = 0;
	if ( qApp->activeWindow() )
		id = qApp->activeWindow()->winId();
	QString password;
	Wallet* wallet = Wallet::openWallet ( Wallet::NetworkWallet(), id );
	if ( wallet ) {
		QLatin1String folder ( "Knights" );
		if ( !wallet->hasFolder ( folder ) )
			wallet->createFolder ( folder );
		wallet->setFolder ( folder );
		QString key = ui->usernameLineEdit->text() + QLatin1Char ( '@' ) + name;
		wallet->readPassword ( key, password );
	} else
		qCDebug(LOG_KNIGHTS) << "KWallet not available";
	ui->passwordLineEdit->setText ( password );
	serverName = name;
}

void FicsDialog::setConsoleWidget ( QWidget* widget ) {
	ui->tabWidget->widget ( 4 )->layout()->addWidget ( widget );
}

void FicsDialog::focusOnLogin() {
	ui->tabWidget->setCurrentIndex ( 0 );
}

void FicsDialog::setStatus ( const QString& status, bool error ) {
	if ( error )
		ui->logInStatusLabel->setText ( i18n ( "<font color='red'>Error: %1</font>", status ) );
	else
		ui->logInStatusLabel->setText ( status );
}

bool FicsDialog::remember() {
	return ui->rememberCheckBox->isChecked();
}

void FicsDialog::saveFicsSettings() {
	Settings::setAutoLogin(ui->rememberCheckBox->isChecked());

	Settings::setFicsUsername ( ui->usernameLineEdit->text() );
	Settings::setGuest ( !ui->registeredCheckBox->isChecked() );

	WId id = 0;
	if ( qApp->activeWindow() )
		id = qApp->activeWindow()->winId();
	Wallet* wallet = Wallet::openWallet ( Wallet::NetworkWallet(), id );
	if ( wallet ) {
		QLatin1String folder ( "Knights" );
		if ( !wallet->hasFolder ( folder ) )
			wallet->createFolder ( folder );
		wallet->setFolder ( folder );
		QString key = ui->usernameLineEdit->text() + QLatin1Char ( '@' ) + serverName;
		wallet->writePassword ( key, ui->passwordLineEdit->text() );
	}
	Settings::self()->save();
}

void FicsDialog::removeGameOffer ( int id ) {
	if (!m_gameId.contains(id))
		return;
	ui->offerTable->removeRow(m_gameId.indexOf(id));
	ui->graphView->removeSeek(id);
	m_gameId.removeAll(id);
}

void FicsDialog::setLoginEnabled ( bool enable ) {
	ui->logInButton->setEnabled ( enable );
}

void FicsDialog::removeChallenge ( int id ) {
	m_challengeModel.removeRows ( m_challengeId.indexOf(id), 1 );
	m_challengeId.removeAll(id);
}

bool FicsDialog::autoAcceptChallenge() {
	return ui->autoCheckBox->isChecked();
}

bool FicsDialog::rated() {
	return ui->ratedCheckBox->isChecked();
}

void FicsDialog::rememberCheckBoxChanged( int state ) {
	Q_UNUSED(state)
	Settings::setAutoLogin(ui->rememberCheckBox->isChecked());
	Settings::self()->save();
}
