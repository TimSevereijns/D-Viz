#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>

#include "aboutDialog.h"
#include "breakdownDialog.h"
#include "constants.h"
#include "controller.h"
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

   friend class Controller;

   public:

      /**
       * @brief MainWindow
       *
       * @param[in] controller
       * @param[in] parent
       */
      MainWindow(
         Controller& controller,
         QWidget* parent = nullptr);

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
       * @brief Sets a temporary message in the status bar.
       *
       * @param[in] message         The message to display.
       * @param[in] timeout         Duration of the message in milliseconds.
       */
      void SetStatusBarMessage(
         const std::wstring& message,
         int timeout = 0);

      /**
       * @returns True if the frame time readout should be shown in the titlebar.
       */
      bool ShouldShowFrameTime() const;

      /**
       * @returns The current search query.
       */
      std::wstring GetSearchQuery() const;

      /**
       * @brief GetModel
       * @return
       */
      Controller& GetController();

      /**
       * @brief GetCanvas
       * @return
       */
      GLCanvas& GetCanvas();

   public slots:

      /**
       * @brief OnFileMenuNewScan
       */
      void OnFileMenuNewScan();

      /**
       * @brief OnFPSReadoutToggled
       */
      void OnFPSReadoutToggled(bool isEnabled);


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

      void ScanDrive(VisualizationParameters& vizParameters);

      /**
       * @brief Prompts the user if he or she would like to set a lower bound on which files are
       * visualized.
       *
       * @param[in] numberOfFilesScanned     The number of scanned files that can be visualized.
       * @param[in] parameters               @see VisualizationParameters
       */
      void AskUserToLimitFileSize(
         const std::uintmax_t numberOfFilesScanned,
         VisualizationParameters& parameters);

      void SetFilePruningComboBoxValue(uintmax_t minimum);

      void ComputeProgress(
         const std::uintmax_t numberOfFilesScanned,
         const std::uintmax_t bytesProcessed);

      void CreateMenus();
      void CreateFileMenu();
      void CreateViewMenu();
      void CreateHelpMenu();

      void LaunchAboutDialog();

      void SetupSidebar();
      void SetupXboxController();

      Controller& m_controller;

      DriveScanner m_scanner;

      bool m_showDirectoriesOnly{ false };
      bool m_useDirectoryGradient{ false };
      bool m_xboxControllerConnected{ false };

      int m_sizePruningComboBoxIndex{ 0 };

      std::uint64_t m_occupiedDiskSpace{ 0 };

      std::wstring m_searchQuery{ };

      std::unique_ptr<XboxController> m_xboxController{ new XboxController };
      std::unique_ptr<XboxController::State> m_xboxControllerState{ new XboxController::State };

      std::unique_ptr<QMenu> m_fileMenu{ nullptr };
      std::unique_ptr<QMenu> m_viewMenu{ nullptr };
      std::unique_ptr<QMenu> m_helpMenu{ nullptr };

      std::unique_ptr<QAction> m_fileMenuNewScan{ nullptr };
      std::unique_ptr<QAction> m_fileMenuPreferences{ nullptr };
      std::unique_ptr<QAction> m_fileMenuExit{ nullptr };
      std::unique_ptr<QAction> m_viewMenuToggleFrameTime{ nullptr };
      std::unique_ptr<QAction> m_helpMenuAboutDialog{ nullptr };

      std::unique_ptr<GLCanvas> m_glCanvas{ nullptr };

      std::unique_ptr<AboutDialog> m_aboutDialog{ nullptr };

      std::unique_ptr<BreakdownDialog> m_breakdownDialog{ nullptr };

      std::shared_ptr<OptionsManager> m_optionsManager{ nullptr };

      Ui::MainWindow* m_ui{ nullptr };

      std::wstring m_directoryToVisualize{ L"" };

      std::vector<std::pair<std::uintmax_t, QString>> m_sizePruningOptions
      {
         std::pair<std::uintmax_t, QString>(0,                                      "Show All"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_KIBIBYTE,       "< 1 Kib"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_MEBIBYTE,       "< 1 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_MEBIBYTE * 10,  "< 10 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_MEBIBYTE * 100, "< 100 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_MEBIBYTE * 250, "< 250 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_MEBIBYTE * 500, "< 500 MiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_GIBIBYTE,       "< 1 GiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_GIBIBYTE * 5,   "< 5 GiB"),
         std::pair<std::uintmax_t, QString>(Constants::FileSize::ONE_GIBIBYTE * 10,  "< 10 GiB")
      };
};

#endif // MAINWINDOW_H
