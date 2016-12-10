#ifndef BREAKDOWNDIALOG_H
#define BREAKDOWNDIALOG_H

#include <QDialog>

#include "scanBreakdownModel.h"

#include <functional>

namespace Ui
{
   class breakdownDialog;
}

/**
 * @brief The BreakdownDialog class
 */
class BreakdownDialog : public QDialog
{
      Q_OBJECT

   public:

      explicit BreakdownDialog(QWidget* parent = nullptr);

      ~BreakdownDialog();

   private:

      Ui::breakdownDialog* m_ui;

      ScanBreakdownModel m_model;
};

#endif // BREAKDOWNDIALOG_H
