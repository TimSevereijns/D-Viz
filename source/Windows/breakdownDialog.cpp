#include "breakdownDialog.h"
#include "ui_breakdownDialog.h"

BreakdownDialog::BreakdownDialog(QWidget* parent) :
   QDialog(parent),
   ui(new Ui::breakdownDialog)
{
   ui->setupUi(this);
}

BreakdownDialog::~BreakdownDialog()
{
   delete ui;
}
