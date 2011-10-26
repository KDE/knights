#ifndef KNIGHTS_HISTORYWIDGET_H
#define KNIGHTS_HISTORYWIDGET_H

#include <QWidget>
#include "core/move.h"

class QStringListModel;

namespace Ui {
  class HistoryWidget;
}

namespace Knights {

class HistoryWidget : public QWidget
{
  Q_OBJECT

public:
    explicit HistoryWidget(QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~HistoryWidget();
    
private:
    Ui::HistoryWidget* ui;
    QStringListModel* model;
    
private slots:
    void updateModel();
    void updateModelStandardNotation ( Move::Notation notation );
};

}

#endif // KNIGHTS_HISTORYWIDGET_H
