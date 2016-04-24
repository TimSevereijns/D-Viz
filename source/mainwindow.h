#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>

#include "constants.h"
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
       * @brief OnDirectoryGradientStateChanged
       * @param state
       */
      void OnDirectoryGradientStateChanged(int state);

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

      void SetupSidebar();
      void SetupXboxController();

      bool m_showDirectoriesOnly{ false };
      bool m_useDirectoryGradient{ false };
      bool m_xboxControllerConnected{ false };

      int m_sizePruningComboBoxIndex{ 0 };

      std::unique_ptr<XboxController> m_xboxController{ new XboxController{ 0 } };
      std::unique_ptr<XboxController::State> m_xboxControllerState{ nullptr };

      std::unique_ptr<QMenu> m_fileMenu{ nullptr };

      std::unique_ptr<QAction> m_fileMenuNewScan{ nullptr };
      std::unique_ptr<QAction> m_fileMenuPreferences{ nullptr };
      std::unique_ptr<QAction> m_fileMenuExit{ nullptr };

      std::unique_ptr<GLCanvas> m_glCanvas{ nullptr };

      std::shared_ptr<OptionsManager> m_optionsManager{ nullptr };

      Ui::MainWindow* m_ui{ nullptr };

      std::wstring m_directoryToVisualize{ L"" };

      std::vector<std::pair<std::uintmax_t, QString>> m_sizePruningOptions
      {
         std::pair<std::uintmax_t, QString>(0,                                      "Show All"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneKibibyte,       "< 1 Kib"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneMebibyte,       "< 1 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneMebibyte * 10,  "< 10 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneMebibyte * 100, "< 100 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneMebibyte * 250, "< 250 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneMebibyte * 500, "< 500 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneGibibyte,       "< 1 GiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneGibibyte * 5,   "< 5 GiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::oneGibibyte * 10,  "< 10 GiB")
      };
};

#endif // MAINWINDOW_H
