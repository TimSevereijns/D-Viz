#ifndef BREAKDOWNDIALOG_H
#define BREAKDOWNDIALOG_H

#include <QDialog>
#include <QSortFilterProxyModel>

#include <functional>
#include <memory>

#include "scanBreakdownModel.h"

class MainWindow;

namespace Ui
{
   class breakdownDialog;
}

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

      BreakdownDialog(QWidget* parent = nullptr);

      ~BreakdownDialog();

      void ReloadData();

   protected:

      void resizeEvent(QResizeEvent* event) override;

   private:

      void AdjustColumnWidthsToFitViewport();

      MainWindow& m_mainWindow;

      std::unique_ptr<Ui::breakdownDialog> m_ui;

      ScanBreakdownModel m_model;
      ScanBreakdownFilterProxyModel m_proxyModel;
};

#endif // BREAKDOWNDIALOG_H
