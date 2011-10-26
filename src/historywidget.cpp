#include "historywidget.h"

#include "ui_historywidget.h"
#include "core/move.h"
#include "gamemanager.h"
#include <QStringListModel>
#include <KDebug>
#include <QScrollBar>

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





