#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
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

struct ScanningProgress;

class MainWindow final : public QMainWindow
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
       * @returns The base directory of the visualization.
       */
      std::wstring GetDirectoryToVisualize() const;

      /**
       * @brief Sets the field of view.
       *
       * @param fieldOfView
       */
      void SetFieldOfViewSlider(int fieldOfView);

      /**
       * @brief Sets the camera movement speed.
       *
       * @param speed
       */
      void SetCameraSpeedSpinner(double speed);

      /**
       * @returns The options manager.
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
       * @brief GetController
       *
       * @return
       */
      Controller& GetController();

      /**
       * @brief GetCanvas
       *
       * @return
       */
      GLCanvas& GetCanvas();

      /**
       * @brief GetGamepad
       *
       * @return
       */
      Gamepad& GetGamepad();

   private slots:

      void OnFileMenuNewScan();

      void OnFPSReadoutToggled(bool isEnabled);

      void SwitchToBinaryPrefix(bool useBinary);

      void SwitchToDecimalPrefix(bool useDecimal);

      void OnNewSearchQuery();

      void PruneTree();

      void OnFieldOfViewChange(int fieldOfView);

      void OnDirectoryPruningChange(int state);

      void OnGradientUseChange(int state);

      void OnShowBreakdownButtonPressed();

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
         std::uintmax_t numberOfFilesScanned,
         VisualizationParameters& parameters);

      void SetFilePruningComboBoxValue(std::uintmax_t minimum);

      void ComputeProgress(const ScanningProgress& progress);

      void LaunchAboutDialog();

      void SetupMenus();
      void SetupFileSizePruningDropdown();
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

      int m_sizePruningComboBoxIndex{ 0 };

      std::uint64_t m_occupiedDiskSpace{ 0u };

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

      Ui::MainWindow m_ui{ };

      std::unique_ptr<GLCanvas> m_glCanvas{ nullptr };

      std::unique_ptr<AboutDialog> m_aboutDialog{ nullptr };

      std::unique_ptr<BreakdownDialog> m_breakdownDialog{ nullptr };

      std::shared_ptr<OptionsManager> m_optionsManager{ nullptr };

      std::wstring m_searchQuery{ };

      std::experimental::filesystem::path m_rootPath{ };

      const std::vector<std::pair<std::uintmax_t, QString>>* m_fileSizeOptions{ nullptr };
};

#endif // MAINWINDOW_H
