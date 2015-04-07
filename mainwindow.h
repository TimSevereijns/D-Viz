#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMenu>

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
      void CreateMenus();

      QMenu* fileMenu;

      Ui::MainWindow* ui;

      std::wstring m_launchArgPath;
};

#endif // MAINWINDOW_H
