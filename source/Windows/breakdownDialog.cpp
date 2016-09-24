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

   for (auto&& node : tree)
   {
      m_model.insert(&node);
   }

   m_ui->treeView->setModel(&m_model);
}

BreakdownDialog::~BreakdownDialog()
{
   delete m_ui;
}
