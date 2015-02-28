#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
   class MainWindow;
}

class GLCanvas;

class MainWindow : public QMainWindow
{
   Q_OBJECT

   public:
      explicit MainWindow(QWidget* parent = 0);
      ~MainWindow();

   protected:
      GLCanvas* m_glCanvas;

      void resizeEvent(QResizeEvent* event) override;

   private:
      Ui::MainWindow* ui;
};

#endif // MAINWINDOW_H
