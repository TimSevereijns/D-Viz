#include "aboutDialog.h"
#include "ui_aboutDialog.h"

AboutDialog::AboutDialog(QWidget *parent) :
   QDialog{ parent },
   m_ui{ std::make_unique<Ui::AboutDialog>() }
{
   m_ui->setupUi(this);
}
