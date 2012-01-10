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

#include "historywidget.h"

#include "ui_historywidget.h"
#include "core/move.h"
#include "gamemanager.h"
#include <QStringListModel>
#include <KDebug>
#include <QScrollBar>
#include <KStandardGameAction>
#include <KActionCollection>
#include "knights.h"

using namespace Knights;

HistoryWidget::HistoryWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  model = new QStringListModel(this);
  
  ui = new Ui::HistoryWidget;
  ui->setupUi ( this );
  ui->listView->setModel ( model );
  
  connect ( ui->notationComboBox, SIGNAL(currentIndexChanged(int)), SLOT(updateModel()));
  connect ( Manager::self(), SIGNAL(historyChanged()), SLOT(updateModel()) );
  
  ui->saveButton->setIcon ( KIcon(QLatin1String("document-save")) );
  connect ( ui->saveButton, SIGNAL(clicked(bool)), SLOT(saveHistory()));
  
  kDebug();
}

HistoryWidget::~HistoryWidget()
{
  delete ui;
}

void HistoryWidget::updateModel()
{
  switch ( ui->notationComboBox->currentIndex() )
  {
    case 0:
      updateModelStandardNotation ( Move::Algebraic );
      break;
      
    case 1:
      updateModelStandardNotation ( Move::LongAlgebraic );
      break;
      
    case 2:
      updateModelStandardNotation ( Move::Coordinate );
      break;
      
    default:
      updateModelStandardNotation ( Move::Algebraic );
      break;
  }
}

void HistoryWidget::updateModelStandardNotation ( Move::Notation notation )
{
  bool bottom = ui->listView->verticalScrollBar()->value() == ui->listView->verticalScrollBar()->maximum();
  QStringList list;
  int i = 0;
  foreach ( const Move& move, Manager::self()->moveHistory() )
  {
    QString string = QString::number ( i/2 + 1 ) + QLatin1String(". ");
    if ( i % 2 )
    {
      string += QLatin1String("... ");
    }
    list << string + move.stringForNotation ( notation );
    ++i;
  }
  model->setStringList ( list );
  
  if ( bottom )
  {
    ui->listView->scrollToBottom();
  }
}

void HistoryWidget::saveHistory()
{
  MainWindow* mw = qobject_cast<MainWindow*>(window());
  mw->fileSave();
}






