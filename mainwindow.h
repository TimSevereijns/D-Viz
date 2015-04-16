#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QMainWindow>
#include <QMenu>

#include <memory>
#include <cstdint>
#include <string>

namespace Ui
{
   class MainWindow;
}

class GLCanvas;

class MainWindow : public QMainWindow
{
   Q_OBJECT

   public:
      explicit MainWindow(QWidget* parent = 0, std::wstring path = L"");

      ~MainWindow();

      std::wstring GetDirectoryToVisualize() const;

   public slots:
      void OnFileMenuNewScan();
      void OnFieldOfViewChanged(int fieldOfView);
      void OnDirectoryOnlyStateChanged(int state);
      void OnPruneTreeButtonClicked();
      void OnPruneSizeComboBoxSelectionChanged(int index);

   private:
      void CreateMenus();
      void CreateFileMenu();
      void CreateWindowMenu();

      void SetupSidebar();

      bool m_showDirectoriesOnly;

      int m_sizePruningComboBoxIndex;

      std::unique_ptr<QMenu> m_fileMenu;

      std::unique_ptr<QAction> m_fileMenuNewScan;
      std::unique_ptr<QAction> m_fileMenuPreferences;
      std::unique_ptr<QAction> m_fileMenuExit;

      std::unique_ptr<GLCanvas> m_glCanvas;

      Ui::MainWindow* ui; // Unfortunately it would appear as if this has to remain a raw pointer.

      std::wstring m_directoryToVisualize;

      std::vector<std::pair<std::uintmax_t, std::string>> m_sizePruningOptions;
};

#endif // MAINWINDOW_H
