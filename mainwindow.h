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
      explicit MainWindow(QWidget* parent = 0);

      ~MainWindow();

      /**
       * @brief GetDirectoryToVisualize
       * @return
       */
      std::wstring GetDirectoryToVisualize() const;

      /**
       * @brief UpdateFieldOfViewSlider
       * @param fieldOfView
       */
      void UpdateFieldOfViewSlider(const int fieldOfView);

   public slots:
      /**
       * @brief OnFileMenuNewScan
       */
      void OnFileMenuNewScan();

      /**
       * @brief OnFieldOfViewChanged
       * @param fieldOfView
       */
      void OnFieldOfViewChanged(int fieldOfView);

      /**
       * @brief OnDirectoryOnlyStateChanged
       * @param state
       */
      void OnDirectoryOnlyStateChanged(int state);

      /**
       * @brief OnPruneTreeButtonClicked
       */
      void OnPruneTreeButtonClicked();

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

      Ui::MainWindow* m_ui; // Unfortunately it would appear as if this has to remain a raw pointer.

      std::wstring m_directoryToVisualize;

      std::vector<std::pair<std::uintmax_t, QString>> m_sizePruningOptions;
};

#endif // MAINWINDOW_H
