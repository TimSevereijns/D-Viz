#ifndef BREAKDOWNDIALOG_H
#define BREAKDOWNDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>

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

   protected:

      void resizeEvent(QResizeEvent* event) final override;

   private:

      void AdjustColumnWidthsToFitViewport();

      Ui::breakdownDialog* m_ui;

      ScanBreakdownModel m_model;
      QSortFilterProxyModel m_proxyModel;
};

#endif // BREAKDOWNDIALOG_H
