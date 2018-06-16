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
       * @brief Sets the field of view.
       *
       * @note This function will update both the UI as well as the backing value.
       *
       * @param[in] fieldOfView     The new value to set the field of view to.
       */
      void SetFieldOfViewSlider(int fieldOfView);

      /**
       * @brief Sets the camera movement speed.
       *
       * @note This function will update both the UI as well as the backing value.
       *
       * @param[in] speed           The new value to set the camera's speed to.
       */
      void SetCameraSpeedSpinner(double speed);

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
       * @brief ReloadVisualization
       */
      void ReloadVisualization();

      /**
       * @returns True if the frame time readout should be shown in the titlebar.
       */
      bool ShouldShowFrameTime() const;

      /**
       * @returns The current search query.
       */
      std::wstring GetSearchQuery() const;

      /**
       * @returns A reference to the model controller for the treemap visualization.
       */
      Controller& GetController();

      /**
       * @returns A reference to the OpenGL canvas.
       */
      GLCanvas& GetCanvas();

      /**
       * @returns A reference to the gamepad instance.
       */
      Gamepad& GetGamepad();

   private slots:

      void OnScanStarted();

      void OnScanCompleted();

      void OnFileMenuNewScan();

      void OnFPSReadoutToggled(bool isEnabled);

      void SwitchToBinaryPrefix(bool useBinary);

      void SwitchToDecimalPrefix(bool useDecimal);

      void OnNewSearchQuery();

      void OnSearchQueryTextChanged(const QString& text);

      void OnApplyButtonPressed();

      void OnFieldOfViewChange(int fieldOfView);

      void OnDirectoryPruningChange(int state);

      void OnGradientUseChange(int state);

      void OnShowBreakdownButtonPressed();

      void OnRenderOriginToggled(bool isEnabled);

      void OnRenderGridToggled(bool isEnabled);

      void OnRenderLightMarkersToggled(bool isEnabled);

      void OnRenderFrustumToggled(bool isEnabled);

   private:

      bool AskUserToLimitFileSize(
         std::uintmax_t numberOfFilesScanned,
         Settings::VisualizationParameters& parameters);

      void SetWaitCursor();

      void RestoreDefaultCursor();

      void SetFilePruningComboBoxValue(std::uintmax_t minimum);

      void PruneTree();

      void ApplyColorScheme();

      void LaunchAboutDialog();

      void SetupMenus();
      void SetupColorSchemeDropdown();
      void SetupFileSizePruningDropdown();
      void SetupFileMenu();
      void SetupFileSizeSubMenu();
      void SetupOptionsMenu();
      void SetupDebuggingMenu();
      void SetupHelpMenu();
      void SetupSidebar();
      void SetupGamepad();

      void SetDebuggingMenuState();

      Controller& m_controller;

      bool m_showDirectoriesOnly{ false };
      bool m_useDirectoryGradient{ false };

      int m_sizePruningComboBoxIndex{ 0 };

      std::unique_ptr<Gamepad> m_gamepad{ std::make_unique<Gamepad>(0, this) };

      Ui::MainWindow m_ui;

      std::unique_ptr<GLCanvas> m_glCanvas{ nullptr };
      std::unique_ptr<AboutDialog> m_aboutDialog{ nullptr };
      std::unique_ptr<BreakdownDialog> m_breakdownDialog{ nullptr };

      std::wstring m_searchQuery;

      const std::vector<std::pair<std::uintmax_t, QString>>* m_fileSizeOptions{ nullptr };

      // @note The remainder of this header is dedicated to the various menus that exist within
      // the main window. Since some of these menus are submenus of other menus, the variable
      // declaration order is critical to ensuring proper lifetime management. In other words,
      // be careful in modifying this section; any errors likely won't show up until the program
      // exits.

      QMenu m_fileMenu{ nullptr };

      struct FileMenu
      {
         QAction newScan{ nullptr };
         QAction exit{ nullptr };
      } m_fileMenuWrapper;

      QMenu m_optionsMenu{ nullptr };

      struct OptionsMenu
      {
         QAction toggleFrameTime{ nullptr };

         QMenu fileSizeMenu{ nullptr };

         struct FileSizeMenu
         {
            QAction binaryPrefix{ nullptr };
            QAction decimalPrefix{ nullptr };
         } fileSizeMenuWrapper;
      } m_optionsMenuWrapper;

      QMenu m_debuggingMenu{ nullptr };

      struct DebuggingMenu
      {
         QMenu renderMenu{ nullptr };

         struct RenderMenuWrapper
         {
            QAction origin{ nullptr };
            QAction grid{ nullptr };
            QAction lightMarkers{ nullptr };
            QAction frustum{ nullptr };
         } renderMenuWrapper;

         QMenu lightingMenu{ nullptr };

         struct LightingMenuWrapper
         {
            QAction showCascadeSplits{ nullptr };
            QAction showShadows{ nullptr };
         } lightingMenuWrapper;
      } m_debuggingMenuWrapper;

      QMenu m_helpMenu{ nullptr };

      struct HelpMenu
      {
         QAction aboutDialog{ nullptr };
      } m_helpMenuWrapper;
};

#endif // MAINWINDOW_H
