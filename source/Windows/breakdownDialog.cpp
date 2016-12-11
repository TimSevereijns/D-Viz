#include "breakdownDialog.h"
#include "ui_breakdownDialog.h"

#include "mainWindow.h"

#include <QResizeEvent>
#include <QScrollbar>

#include <functional>
#include <iostream>
#include <vector>

BreakdownDialog::BreakdownDialog(QWidget* parent) :
   QDialog{ parent },
   m_ui{ new Ui::breakdownDialog }
{
   assert(m_ui);
   m_ui->setupUi(this);

   auto* mainWindow = reinterpret_cast<MainWindow*>(parent);
   assert(mainWindow);

   const auto& controller = mainWindow->GetController();
   const auto tree = controller.GetTree();

   if (tree.GetHead()->GetChildCount() == 0)
   {
      return;
   }

   for (const auto& node : tree)
   {
      m_model.insert(node);
   }

   m_proxyModel.setSourceModel(&m_model);
   m_ui->tableView->setModel(&m_proxyModel);

   m_ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
   m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
   m_ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
   m_ui->tableView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
   m_ui->tableView->setSortingEnabled(true);

   AdjustColumnWidthsToFitViewport();
}

void BreakdownDialog::AdjustColumnWidthsToFitViewport()
{
   const auto headerWidth = m_ui->tableView->verticalHeader()->width();

   const auto scrollbarWidth = m_ui->tableView->verticalScrollBar()->isVisible()
      ? m_ui->tableView->verticalScrollBar()->width()
      : 0;

   const auto tableWidth = m_ui->tableView->width() - headerWidth - scrollbarWidth;

   m_ui->tableView->setColumnWidth(0, tableWidth / 2);
   m_ui->tableView->setColumnWidth(1, tableWidth / 2);
}

void BreakdownDialog::resizeEvent(QResizeEvent* /*event*/)
{
   AdjustColumnWidthsToFitViewport();
}

BreakdownDialog::~BreakdownDialog()
{
   delete m_ui;
}
