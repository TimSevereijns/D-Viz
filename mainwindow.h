#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include <string>

namespace Ui {
   class MainWindow;
}

class GLCanvas;

class MainWindow : public QMainWindow
{
   Q_OBJECT

   public:
      explicit MainWindow(QWidget* parent = 0, std::wstring path = L"C:\\Users\\Tim");

      ~MainWindow();

      std::wstring GetLaunchArgPath() const;

   protected:
      GLCanvas* m_glCanvas;

   private:
      Ui::MainWindow* ui;
      std::wstring m_launchArgPath;
};

#endif // MAINWINDOW_H
