#include "breakdownDialog.h"
#include "ui_breakdownDialog.h"

#include "mainWindow.h"

#include <functional>
#include <iostream>
#include <vector>

BreakdownDialog::BreakdownDialog(QWidget* parent) :
   QDialog{ parent },
   m_ui{ new Ui::breakdownDialog }
{
   m_ui->setupUi(this);

   auto* mainWindow = reinterpret_cast<MainWindow*>(parent);
   assert(mainWindow);

   const auto& controller = mainWindow->GetController();
   const auto tree = controller.GetTree();

   for (const auto& node : tree)
   {
      m_model.insert(node);
   }

   //m_model.setHeaderData(0, Qt::Orientation::Horizontal, QString("File Type"));
   //m_model.setHeaderData(1, Qt::Orientation::Horizontal, QString("Cumulative Size (Bytes)"));

   m_ui->tableView->setModel(&m_model);

   m_ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
   m_ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
   m_ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

BreakdownDialog::~BreakdownDialog()
{
   delete m_ui;
}
