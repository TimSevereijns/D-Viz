#ifndef ABOUTDIALOG_H
#define ABOUTDIALOG_H

#include <QDialog>

#include <memory>

#include "ui_aboutDialog.h"

class AboutDialog final : public QDialog
{
      Q_OBJECT

   public:

      explicit AboutDialog(QWidget* parent = nullptr);

   private:

      std::unique_ptr<Ui::AboutDialog> m_ui;
};

#endif // ABOUTDIALOG_H
