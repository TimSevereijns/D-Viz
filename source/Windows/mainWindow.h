#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLabel>
#include <QMainWindow>
#include <QMenu>

#include "aboutDialog.h"
#include "breakdownDialog.h"
#include "constants.h"
#include "controller.h"
#include "HID/gamepad.h"
#include "Viewport/glCanvas.h"

#include "ui_mainWindow.h"

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
       * @brief GetOptionsManager
       * @return
       */
      std::shared_ptr<OptionsManager> GetOptionsManager();

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

      /**
       * @brief GetGamepad
       * @return
       */
      Gamepad& GetGamepad();

   public slots:

      /**
       * @brief OnFileMenuNewScan
       */
      void OnFileMenuNewScan();

      /**
       * @brief OnFPSReadoutToggled
       */
      void OnFPSReadoutToggled(bool isEnabled);

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

      void LaunchAboutDialog();

      void SetupMenus();
      void SetupFileMenu();
      void SetupFileSizeSubMenu();
      void SetupOptionsMenu();
      void SetupHelpMenu();
      void SetupSidebar();
      void SetupGamepad();

      Controller& m_controller;

      DriveScanner m_scanner;

      bool m_showDirectoriesOnly{ false };
      bool m_useDirectoryGradient{ false };
      bool m_xboxControllerConnected{ false };

      int m_sizePruningComboBoxIndex{ 0 };

      std::uint64_t m_occupiedDiskSpace{ 0 };

      std::unique_ptr<Gamepad> m_gamepad{ std::make_unique<Gamepad>(0, this) };

      QMenu m_fileMenu{ nullptr };

      /**
       * @brief Wraps everything that constitutes the "File" menu.
       */
      struct FileMenu
      {
         QAction newScan{ nullptr };
         QAction exit{ nullptr };
      } m_fileMenuWrapper;

      // @note Since any sub-menus of this menu will reference this menu as a parent, it's
      // imperative that this menu outlive any of its sub-menus. In other words, make sure that
      // this menu is declared before any of its sub-menus in this class.
      QMenu m_optionsMenu{ nullptr };

      /**
       * @brief Wraps everything that constitutes the "Options" menu.
       */
      struct OptionsMenu
      {
         QMenu fileSizeMenu{ nullptr };

         struct FileSizeMenu
         {
            QAction binaryPrefix{ nullptr };
            QAction decimalPrefix{ nullptr };
         } fileSizeMenuWrapper;

         QAction toggleFrameTime{ nullptr };
      } m_optionsMenuWrapper;

      QMenu m_helpMenu{ nullptr };

      /**
       * @brief Wraps everything that constitutes the "Help" menu.
       */
      struct HelpMenu
      {
         QAction aboutDialog{ nullptr };
      } m_helpMenuWrapper;

      std::unique_ptr<GLCanvas> m_glCanvas{ nullptr };

      std::unique_ptr<AboutDialog> m_aboutDialog{ nullptr };

      std::unique_ptr<BreakdownDialog> m_breakdownDialog{ nullptr };

      std::unique_ptr<Ui::MainWindow> m_ui{ nullptr };

      std::shared_ptr<OptionsManager> m_optionsManager{ nullptr };

      std::wstring m_searchQuery{ };
      std::wstring m_directoryToVisualize{ };

      std::vector<std::pair<std::uintmax_t, QString>> m_sizePruningOptions
      {
         std::pair<std::uintmax_t, QString>(0,                                       "Show All"),
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
