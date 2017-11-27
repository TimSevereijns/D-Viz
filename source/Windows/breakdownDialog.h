#ifndef BREAKDOWNDIALOG_H
#define BREAKDOWNDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>

#include "scanBreakdownModel.h"

#include <functional>

#include "ui_breakdownDialog.h"

class ScanBreakdownFilterProxyModel final : public QSortFilterProxyModel
{
   bool lessThan(
      const QModelIndex& lhs,
      const QModelIndex& rhs) const override
   {
      const auto lhsData = sourceModel()->data(lhs, Qt::UserRole);
      const auto rhsData = sourceModel()->data(rhs, Qt::UserRole);

      if (lhs.column() == 0)
      {
         const auto lhsExtension = lhsData.value<QString>();
         const auto rhsExtension = rhsData.value<QString>();

         return lhsExtension < rhsExtension;
      }

      if (lhs.column() == 1)
      {
         const auto lhsSize = lhsData.value<std::uintmax_t>();
         const auto rhsSize = rhsData.value<std::uintmax_t>();

         return lhsSize < rhsSize;
      }

      assert(false);
      return false;
   }
};

/**
 * @brief The BreakdownDialog class
 */
class BreakdownDialog final : public QDialog
{
      Q_OBJECT

   public:

      explicit BreakdownDialog(QWidget* parent = nullptr);

   protected:

      void resizeEvent(QResizeEvent* event) override;

   private:

      void AdjustColumnWidthsToFitViewport();

      MainWindow& m_mainWindow;

      Ui::breakdownDialog m_ui;

      ScanBreakdownModel m_model;
      ScanBreakdownFilterProxyModel m_proxyModel;
};

#endif // BREAKDOWNDIALOG_H
