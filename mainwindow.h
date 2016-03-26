#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>

#include "HID/xboxController.h"

namespace Ui
{
   class MainWindow;
}

class GLCanvas;
class OptionsManager;

class MainWindow : public QMainWindow
{
   Q_OBJECT

   public:
      explicit MainWindow(QWidget* parent = nullptr);

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
      void SetFieldOfViewSlider(int fieldOfView);

      /**
       * @brief UpdateCameraSpeedComboBox
       * @param speed
       */
      void SetCameraSpeedSpinner(double speed);

      /**
       * @brief SetFilePruningComboBox
       * @param index
       */
      void SetFilePruningComboBoxValue(uintmax_t minimum);

      /**
       * @brief GetXboxController
       * @return
       */
      XboxController::State& GetXboxControllerState() const;

      /**
       * @brief GetOptionsManager
       * @return
       */
      std::shared_ptr<OptionsManager> GetOptionsManager();

      /**
       * @brief GetXboxControllerManager
       * @return
       */
      XboxController& GetXboxControllerManager();

      /**
       * @brief SetPermanentStatusBarMessage
       * @param message
       */
      void SetStatusBarMessage(const std::wstring& message);

   public slots:
      /**
       * @brief OnFileMenuNewScan
       */
      void OnFileMenuNewScan();

      /**
       * @brief OnFieldOfViewChanged
       * @param fieldOfView
       *
       * @todo Move to GLCanvas...
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

      /**
       * @brief XboxControllerConnected
       */
      void XboxControllerConnected();

      /**
       * @brief XboxControllerDisconnect
       */
      void XboxControllerDisconnected();

      /**
       * @brief IsXboxControllerConnected
       * @return
       */
      bool IsXboxControllerConnected() const;

      /**
       * @brief XboxControllerStateChanged
       * @param state
       */
      void XboxControllerStateChanged(XboxController::State state);

   private:
      void CreateMenus();
      void CreateFileMenu();
      void CreateWindowMenu();

      void SetupSidebar();
      void SetupXboxController();

      bool m_showDirectoriesOnly;
      bool m_xboxControllerConnected;

      int m_sizePruningComboBoxIndex;

      std::unique_ptr<XboxController> m_xboxController;
      std::unique_ptr<XboxController::State> m_xboxControllerState;

      std::unique_ptr<QMenu> m_fileMenu;

      std::unique_ptr<QAction> m_fileMenuNewScan;
      std::unique_ptr<QAction> m_fileMenuPreferences;
      std::unique_ptr<QAction> m_fileMenuExit;

      std::unique_ptr<QLabel> m_permanentStatusBarMessage;

      std::unique_ptr<GLCanvas> m_glCanvas;

      std::shared_ptr<OptionsManager> m_optionsManager;

      Ui::MainWindow* m_ui;

      std::wstring m_directoryToVisualize;

      std::vector<std::pair<std::uintmax_t, QString>> m_sizePruningOptions;
};

#endif // MAINWINDOW_H
