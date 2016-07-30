#ifndef BREAKDOWNDIALOG_H
#define BREAKDOWNDIALOG_H

#include <QDialog>

namespace Ui
{
   class breakdownDialog;
}

/**
 * @brief The BreakdownDialog class
 *
 * http://doc.qt.io/qt-5/qtwidgets-itemviews-simpletreemodel-example.html
 */
class BreakdownDialog : public QDialog
{
      Q_OBJECT

   public:
      explicit BreakdownDialog(QWidget* parent = nullptr);
      ~BreakdownDialog();

   private:
      Ui::breakdownDialog* ui;
};

#endif // BREAKDOWNDIALOG_H
