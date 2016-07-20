#include "aboutDialog.h"
#include "ui_aboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
   QDialog(parent),
   ui(new Ui::AboutDialog)
{
   ui->setupUi(this);
}

AboutDialog::~AboutDialog()
{
   delete ui;
}
