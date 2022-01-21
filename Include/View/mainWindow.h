#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QLabel>
#include <QMainWindow>
#include <QMenu>
#include <QWindow>

//#if defined(Q_OS_WIN)
//#include <QWinTaskbarButton>
//#include <QWinTaskbarProgress>
//#endif

#include <string_view>

#include "Dialogs/aboutDialog.h"
#include "Dialogs/breakdownDialog.h"
#include "View/HID/gamepad.h"
#include "Viewport/glCanvas.h"
#include "baseView.h"
#include "constants.h"
#include "ui_mainWindow.h"

class Controller;

#if defined(Q_OS_WIN)

// @todo Reimplement. See
// https://code.qt.io/cgit/qt-creator/qt-creator.git/tree/src/plugins/coreplugin/progressmanager

// class WinTaskbarButton : public BaseTaskbarButton
//{
//  public:
//    explicit WinTaskbarButton(QObject* parent) : m_button{ parent }
//    {
//    }

//    void SetWindow(QObject* window) override
//    {
//        m_button.setWindow(static_cast<QWindow*>(window));
//    }

//    void HideProgress() override
//    {
//        m_button.progress()->hide();
//    };

//    void ResetProgress() override
//    {
//        m_button.progress()->reset();
//    }

//    void SetValue(int value) override
//    {
//        m_button.progress()->setValue(value);
//    }

//    void SetMaximum(int value) override
//    {
//        m_button.progress()->setMaximum(value);
//    }

//    void SetMinimum(int value) override
//    {
//        m_button.progress()->setMinimum(value);
//    }

//    void SetVisible(bool value) override
//    {
//        m_button.progress()->setVisible(value);
//    }

//  private:
//    QWinTaskbarButton m_button;
//};

#elif defined(Q_OS_LINUX)

class UnixTaskbarButton : public BaseTaskbarButton
{
  public:
    explicit UnixTaskbarButton(QObject*)
    {
    }
};

#endif // Q_OS_LINUX

class MainWindow final : public QMainWindow, public BaseView
{
    Q_OBJECT

  public:
    /**
     * @brief MainWindow
     *
     * @param[in] controller
     * @param[in] parent
     */
    MainWindow(Controller& controller, QWidget* parent = nullptr);

    /**
     * @brief Shows the main window.
     */
    void Show() override;

    /**
     * @brief GetWindowHandle
     */
    QWindow* GetWindowHandle() override;

    /**
     * @brief Sets the field of view.
     *
     * @note This function will update both the UI as well as the backing value.
     *
     * @param[in] fieldOfView     The new value to set the field of view to.
     */
    void SetFieldOfViewSlider(int fieldOfView) override;

    /**
     * @brief Sets the camera movement speed.
     *
     * @note This function will update both the UI as well as the backing value.
     *
     * @param[in] speed           The new value to set the camera's speed to.
     */
    void SetCameraSpeedSpinner(double speed) override;

    /**
     * @brief Sets a temporary message in the status bar.
     *
     * @param[in] message         The message to display.
     * @param[in] timeout         Duration of the message in milliseconds.
     */
    void SetStatusBarMessage(const std::string& message, int timeout = 0) override;

    /**
     * @brief ReloadVisualization
     */
    void ReloadVisualization() override;

    /**
     * @returns True if the frame time readout should be shown in the titlebar.
     */
    bool ShouldShowFrameTime() const override;

    /**
     * @returns The current search query.
     */
    std::string GetSearchQuery() const override;

    /**
     * @returns A reference to the model controller for the treemap visualization.
     */
    Controller& GetController() override;

    /**
     * @returns A reference to the OpenGL canvas.
     */
    GLCanvas& GetCanvas() override;

    /**
     * @returns A reference to the gamepad instance.
     */
    Gamepad& GetGamepad() override;

    /**
     * @copydoc BaseView::AskUserToLimitFileSize
     */
    bool AskUserToLimitFileSize(std::uintmax_t numberOfFilesScanned) override;

    /**
     * @brief Ask the user to confirm file deletion.
     *
     * @param[in] filePath          The file to be deleted, pending confirmation.
     */
    bool AskUserToConfirmDeletion(const std::filesystem::path& filePath);

    /**
     * @copydoc BaseView::DisplayInfoDialog
     */
    void DisplayInfoDialog(std::string_view message) override;

    /**
     * @copydoc BaseView::DisplayErrorDialog
     */
    void DisplayErrorDialog(std::string_view message) override;

    /**
     * @copydoc BaseView::SetWaitCursor
     */
    void SetWaitCursor() override;

    /**
     * @copydoc BaseView::RestoreDefaultCursor
     */
    void RestoreDefaultCursor() override;

    /**
     * @copydoc BaseView::OnScanStarted
     */
    void OnScanStarted() override;

    /**
     * @copydoc BaseView::OnScanCompleted
     */
    void OnScanCompleted() override;

    /**
     * @copydoc BaseView::GetTaskbarButton
     */
    std::shared_ptr<BaseTaskbarButton> GetTaskbarButton() override;

  private slots:
    void OnFileMenuNewScan();

    void OnFpsReadoutToggled(bool isEnabled);

    void OnShowLightingOptionsToggled(bool isEnabled);

    void OnDarkThemeToggled(bool isEnabled);

    void SwitchToBinaryPrefix(bool useBinary);

    void SwitchToDecimalPrefix(bool useDecimal);

    void OnNewSearchQuery();

    void OnSearchQueryTextChanged(const QString& text);

    void OnApplyButtonPressed();

    void OnFieldOfViewChange(int fieldOfView);

    void OnDirectoryPruningChange(int state);

    void OnShowBreakdownButtonPressed();

    void OnRenderOriginToggled(bool shouldShow);

    void OnRenderGridToggled(bool shouldShow);

    void OnRenderLightMarkersToggled(bool shouldShow);

    void OnRenderFrustaToggled(bool shouldShow);

    void OnShowShadowsToggled(bool shouldShow);

    void OnShowCascadeSplitsToggled(bool shouldShow);

    void OnFileMonitoringToggled(bool shouldEnable);

    void OnOpenLogFile();

    void OnCancelScan();

    void OnClose();

  private:
    void SetFilePruningComboBoxValue(std::uintmax_t minimum);

    void ShowLightingOptions();
    void HideLightingOptions();

    void PruneTree();

    void ApplyColorScheme();

    void LaunchAboutDialog();

    void SetupMenus();
    void SetupColorSchemeDropdown();
    void SetupFileSizePruningDropdown();
    void SetupFileMenu();
    void SetupFileSizeSubMenu();
    void SetupOptionsMenu();
    void SetupLightingSubMenu();
    void SetupRenderSubMenu();
    void SetupDebuggingMenu();
    void SetupHelpMenu();
    void SetupSidebar();
    void SetupGamepad();

    void SetDebuggingMenuState();

    Controller& m_controller;

    bool m_showDirectoriesOnly = false;

    int m_sizePruningComboBoxIndex = 0;

    std::unique_ptr<Gamepad> m_gamepad = std::make_unique<Gamepad>(0, nullptr);

    Ui::MainWindow m_ui;

    std::unique_ptr<GLCanvas> m_glCanvas;
    std::unique_ptr<AboutDialog> m_aboutDialog;
    std::unique_ptr<BreakdownDialog> m_breakdownDialog;

    std::string m_searchQuery;

    const std::vector<std::pair<std::uintmax_t, QString>>* m_fileSizeOptions = nullptr;

    // @note The remainder of this header is dedicated to the various menus that exist within
    // the main window. Since some of these menus are submenus of other menus, the variable
    // declaration order is critical to ensuring proper lifetime management. In other words,
    // be careful in modifying this section; any errors likely won't show up until the program
    // exits.

    QMenu m_fileMenu;

    struct FileMenu
    {
        QAction newScan;
        QAction cancelScan;
        QAction exit;
    } m_fileMenuWrapper;

    QMenu m_optionsMenu;

    struct OptionsMenu
    {
        QAction useDarkTheme;
        QAction enableFileSystemMonitoring;

        QMenu fileSizeMenu;

        struct FileSizeMenu
        {
            QAction binaryPrefix;
            QAction decimalPrefix;
        } fileSizeMenuWrapper;
    } m_optionsMenuWrapper;

    QMenu m_debuggingMenu;

    struct DebuggingMenu
    {
        QMenu renderMenu;

        struct RenderMenuWrapper
        {
            QAction origin;
            QAction grid;
            QAction lightMarkers;
            QAction frustum;
        } renderMenuWrapper;

        QMenu lightingMenu;

        struct LightingMenuWrapper
        {
            QAction showLightingOptions;
            QAction showCascadeSplits;
            QAction showShadows;
        } lightingMenuWrapper;

        QAction openLogFile;
        QAction toggleFrameTime;
    } m_debuggingMenuWrapper;

    QMenu m_helpMenu;

    struct HelpMenu
    {
        QAction aboutDialog;
    } m_helpMenuWrapper;
};

#endif // MAINWINDOW_H
