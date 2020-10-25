#include "View/mainWindow.h"
#include "Model/Scanner/scanningProgress.hpp"
#include "Settings/persistentSettings.h"
#include "Settings/settings.h"
#include "Utilities/operatingSystem.hpp"
#include "Utilities/scopeExit.hpp"
#include "Utilities/scopedCursor.h"
#include "Utilities/utilities.hpp"
#include "View/Viewport/glCanvas.h"
#include "constants.h"
#include "controller.h"
#include "literals.h"

#include <gsl/gsl_assert>
#include <spdlog/spdlog.h>

#include <limits>
#include <memory>

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>
#include <QStandardPaths>

namespace
{
    /**
     * @brief Switches to a different set of entries for the file pruning drop-down menu.
     *
     * @param[in] prefix             The desired unit prefix.
     *
     * @returns A pointer to a const static vector containing the menu values.
     */
    const std::vector<std::pair<std::uintmax_t, QString>>*
    GeneratePruningMenuEntries(Constants::SizePrefix prefix)
    {
        switch (prefix) {
            case Constants::SizePrefix::Decimal: {
                using namespace Literals::Numeric::Decimal;

                // clang-format off
                const static auto decimal = std::vector<std::pair<std::uintmax_t, QString>>{
                    { 0u,       "Show All" },
                    { 1_KB,     "1 KB" },
                    { 10_KB,    "10 KB" },
                    { 100_KB,   "100 KB" },
                    { 1_MB,     "1 MB" },
                    { 10_MB,    "10 MB" },
                    { 100_MB,   "100 MB" },
                    { 250_MB,   "250 MB" },
                    { 500_MB,   "500 MB" },
                    { 1_GB,     "1 GB" },
                    { 5_GB,     "5 GB" },
                    { 10_GB,    "10 GB" }
                };
                // clang-format on

                return &decimal;
            }
            case Constants::SizePrefix::Binary: {
                using namespace Literals::Numeric::Binary;

                // clang-format off
                const static auto binary = std::vector<std::pair<std::uintmax_t, QString>>{
                    { 0u,       "Show All" },
                    { 1_KiB,    "1 KiB" },
                    { 10_KiB,   "10 KiB" },
                    { 100_KiB,  "100 KiB" },
                    { 1_MiB,    "1 MiB" },
                    { 10_MiB,   "10 MiB" },
                    { 100_MiB,  "100 MiB" },
                    { 250_MiB,  "250 MiB" },
                    { 500_MiB,  "500 MiB" },
                    { 1_GiB,    "1 GiB" },
                    { 5_GiB,    "5 GiB" },
                    { 10_GiB,   "10 GiB" }
                };
                // clang-format om

                return &binary;
            }
        }

        GSL_ASSUME(false);
    }
} // namespace

MainWindow::MainWindow(Controller& controller, QWidget* parent /* = nullptr */)
    : QMainWindow{ parent },
      m_controller{ controller },
      m_ui{},
      m_fileSizeOptions{ GeneratePruningMenuEntries(Constants::SizePrefix::Binary) }
{
    m_ui.setupUi(this);

    m_glCanvas = std::make_unique<GLCanvas>(controller, this);
    m_ui.canvasLayout->addWidget(m_glCanvas.get());

    SetupMenus();
    SetupGamepad();
    SetupSidebar();

    SetDebuggingMenuState();
}

void MainWindow::Show()
{
    this->show();
}

QWindow* MainWindow::GetWindowHandle()
{
    return this->windowHandle();
}

void MainWindow::SetupSidebar()
{
    SetupColorSchemeDropdown();
    SetupFileSizePruningDropdown();

    auto& sessionSettings = m_controller.GetSessionSettings();

    connect(
        m_ui.directoriesOnlyCheckBox, &QCheckBox::stateChanged, this,
        &MainWindow::OnDirectoryPruningChange);

    connect(m_ui.applyButton, &QPushButton::clicked, this, &MainWindow::OnApplyButtonPressed);

    connect(m_ui.fieldOfViewSlider, &QSlider::valueChanged, this, &MainWindow::OnFieldOfViewChange);

    connect(m_ui.searchBox, &QLineEdit::returnPressed, this, &MainWindow::OnNewSearchQuery);

    connect(m_ui.searchBox, &QLineEdit::textChanged, this, &MainWindow::OnSearchQueryTextChanged);

    connect(m_ui.searchButton, &QPushButton::clicked, this, &MainWindow::OnNewSearchQuery);

    connect(
        m_ui.showBreakdownButton, &QPushButton::clicked, this,
        &MainWindow::OnShowBreakdownButtonPressed);

    connect(
        m_ui.searchDirectoriesCheckBox, &QCheckBox::stateChanged, &sessionSettings,
        &Settings::SessionSettings::SearchDirectories);

    connect(
        m_ui.searchFilesCheckBox, &QCheckBox::stateChanged, &sessionSettings,
        &Settings::SessionSettings::SearchFiles);

    connect(
        m_ui.cameraSpeedSpinner,
        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        &sessionSettings, &Settings::SessionSettings::SetCameraSpeed);

    connect(
        m_ui.mouseSensitivitySpinner,
        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        &sessionSettings, &Settings::SessionSettings::SetMouseSensitivity);

    connect(
        m_ui.ambientCoefficientSpinner,
        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        &sessionSettings, &Settings::SessionSettings::SetAmbientLightCoefficient);

    connect(
        m_ui.attenuationSpinner,
        static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
        &sessionSettings, &Settings::SessionSettings::SetLightAttenuation);

    connect(
        m_ui.attachLightToCameraCheckBox,
        static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged), &sessionSettings,
        &Settings::SessionSettings::AttachLightToCamera);
}

void MainWindow::SetupColorSchemeDropdown()
{
    m_ui.colorSchemeComboBox->clear();

    const auto& defaultScheme = QString::fromStdWString(Constants::ColorScheme::Default);
    m_ui.colorSchemeComboBox->addItem(defaultScheme, defaultScheme);

    const auto& colorMap = m_controller.GetNodePainter().GetFileColorMap();
    for (const auto& extensionMap : colorMap) {
        const auto& categoryName = QString::fromStdWString(extensionMap.first);
        m_ui.colorSchemeComboBox->addItem(categoryName, categoryName);
    }
}

void MainWindow::SetupFileSizePruningDropdown()
{
    const auto previousIndex = m_ui.minimumSizeComboBox->currentIndex();

    m_ui.minimumSizeComboBox->clear();

    for (const auto& fileSizeAndUnits : *m_fileSizeOptions) {
        m_ui.minimumSizeComboBox->addItem(
            fileSizeAndUnits.second, static_cast<qulonglong>(fileSizeAndUnits.first));
    }

    m_ui.minimumSizeComboBox->setMaxVisibleItems(static_cast<int>(m_fileSizeOptions->size()));
    m_ui.minimumSizeComboBox->setCurrentIndex(previousIndex == -1 ? 0 : previousIndex);

    statusBar()->clearMessage();
}

void MainWindow::SetupGamepad()
{
    const auto gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty()) {
        return;
    }

    m_gamepad = std::make_unique<Gamepad>(*gamepads.begin(), this);
    QGamepadManager::instance()->resetConfiguration(m_gamepad->deviceId());
}

void MainWindow::SetupMenus()
{
    SetupFileMenu();
    SetupOptionsMenu();

    if (m_controller.GetPersistentSettings().ShouldShowDebuggingMenu()) {
        SetupDebuggingMenu();
    }

    SetupHelpMenu();
}

void MainWindow::SetupFileMenu()
{
    m_fileMenuWrapper.newScan.setText("New Scan...");
    m_fileMenuWrapper.newScan.setStatusTip("Start a new visualization.");
    m_fileMenuWrapper.newScan.setShortcuts(QKeySequence::New);

    connect(&m_fileMenuWrapper.newScan, &QAction::triggered, this, &MainWindow::OnFileMenuNewScan);

    m_fileMenuWrapper.exit.setText("Exit");
    m_fileMenuWrapper.exit.setStatusTip("Exit the program.");
    m_fileMenuWrapper.exit.setShortcuts(QKeySequence::Quit);

    connect(&m_fileMenuWrapper.exit, &QAction::triggered, this, &MainWindow::close);

    m_fileMenu.setTitle("File");
    m_fileMenu.addAction(&m_fileMenuWrapper.newScan);
    m_fileMenu.addAction(&m_fileMenuWrapper.exit);

    menuBar()->addMenu(&m_fileMenu);
}

void MainWindow::SetupOptionsMenu()
{
    m_optionsMenuWrapper.toggleFrameTime.setText("Show Frame Time");
    m_optionsMenuWrapper.toggleFrameTime.setStatusTip("Toggle frame-time readout in titlebar.");
    m_optionsMenuWrapper.toggleFrameTime.setCheckable(true);

    connect(
        &m_optionsMenuWrapper.toggleFrameTime, &QAction::toggled, this,
        &MainWindow::OnFPSReadoutToggled);

    m_optionsMenuWrapper.enableFileSystemMonitoring.setText("Monitor File System");
    m_optionsMenuWrapper.enableFileSystemMonitoring.setStatusTip("Monitors the file system for any "
                                                                 "changes");
    m_optionsMenuWrapper.enableFileSystemMonitoring.setCheckable(true);

    const auto isMonitoringEnabled = m_controller.GetPersistentSettings().ShouldMonitorFileSystem();
    m_optionsMenuWrapper.enableFileSystemMonitoring.setChecked(isMonitoringEnabled);

    connect(
        &m_optionsMenuWrapper.enableFileSystemMonitoring, &QAction::toggled, this,
        &MainWindow::OnFileMonitoringToggled);

    m_optionsMenu.setTitle("Options");
    m_optionsMenu.addAction(&m_optionsMenuWrapper.toggleFrameTime);
    m_optionsMenu.addAction(&m_optionsMenuWrapper.enableFileSystemMonitoring);

    SetupFileSizeSubMenu();

    menuBar()->addMenu(&m_optionsMenu);
}

void MainWindow::SetupFileSizeSubMenu()
{
    auto& subMenuWrapper = m_optionsMenuWrapper.fileSizeMenuWrapper;

    subMenuWrapper.binaryPrefix.setText("Binary Prefix");
    subMenuWrapper.binaryPrefix.setStatusTip(
        "Use base-two units, such as kibibytes and mebibytes. This is the default on Windows.");
    subMenuWrapper.binaryPrefix.setCheckable(true);
    subMenuWrapper.binaryPrefix.setChecked(true);

    connect(
        &subMenuWrapper.binaryPrefix, &QAction::toggled, this, &MainWindow::SwitchToBinaryPrefix);

    subMenuWrapper.decimalPrefix.setText("Decimal Prefix");
    subMenuWrapper.decimalPrefix.setStatusTip(
        "Use base-ten units, such as kilobytes and megabytes.");
    subMenuWrapper.decimalPrefix.setCheckable(true);
    subMenuWrapper.decimalPrefix.setChecked(false);

    connect(
        &subMenuWrapper.decimalPrefix, &QAction::toggled, this, &MainWindow::SwitchToDecimalPrefix);

    m_optionsMenuWrapper.fileSizeMenu.setTitle("File Size Units");
    m_optionsMenuWrapper.fileSizeMenu.addAction(&subMenuWrapper.binaryPrefix);
    m_optionsMenuWrapper.fileSizeMenu.addAction(&subMenuWrapper.decimalPrefix);

    m_optionsMenu.addMenu(&m_optionsMenuWrapper.fileSizeMenu);
}

void MainWindow::SetupDebuggingMenu()
{
    auto& renderMenuWrapper = m_debuggingMenuWrapper.renderMenuWrapper;
    auto& renderMenu = m_debuggingMenuWrapper.renderMenu;

    renderMenuWrapper.origin.setText("Origin");
    renderMenuWrapper.origin.setCheckable(true);
    renderMenuWrapper.origin.setChecked(false);

    connect(&renderMenuWrapper.origin, &QAction::toggled, this, &MainWindow::OnRenderOriginToggled);

    renderMenuWrapper.grid.setText("Grid");
    renderMenuWrapper.grid.setCheckable(true);
    renderMenuWrapper.grid.setChecked(false);

    connect(&renderMenuWrapper.grid, &QAction::toggled, this, &MainWindow::OnRenderGridToggled);

    renderMenuWrapper.lightMarkers.setText("Light Markers");
    renderMenuWrapper.lightMarkers.setCheckable(true);
    renderMenuWrapper.lightMarkers.setChecked(false);

    connect(
        &renderMenuWrapper.lightMarkers, &QAction::toggled, this,
        &MainWindow::OnRenderLightMarkersToggled);

    renderMenuWrapper.frustum.setText("Frustum");
    renderMenuWrapper.frustum.setCheckable(true);
    renderMenuWrapper.frustum.setChecked(false);

    connect(
        &renderMenuWrapper.frustum, &QAction::toggled, this, &MainWindow::OnRenderFrustaToggled);

    renderMenu.setTitle("Render Asset");
    renderMenu.setStatusTip("Toggle scene assets on or off");
    renderMenu.addAction(&renderMenuWrapper.origin);
    renderMenu.addAction(&renderMenuWrapper.grid);
    renderMenu.addAction(&renderMenuWrapper.lightMarkers);
    renderMenu.addAction(&renderMenuWrapper.frustum);

    auto& lightingMenuWrapper = m_debuggingMenuWrapper.lightingMenuWrapper;
    auto& lightingMenu = m_debuggingMenuWrapper.lightingMenu;

    const auto shouldShowCascadeSplits =
        m_controller.GetPersistentSettings().ShouldRenderCascadeSplits();

    lightingMenuWrapper.showCascadeSplits.setText("Show Cascade Splits");
    lightingMenuWrapper.showCascadeSplits.setCheckable(true);
    lightingMenuWrapper.showCascadeSplits.setChecked(shouldShowCascadeSplits);

    connect(
        &lightingMenuWrapper.showCascadeSplits, &QAction::toggled, this,
        &MainWindow::OnShowCascadeSplitsToggled);

    const auto shouldShowShadows = m_controller.GetPersistentSettings().ShouldRenderShadows();

    lightingMenuWrapper.showShadows.setText("Show Shadows");
    lightingMenuWrapper.showShadows.setCheckable(true);
    lightingMenuWrapper.showShadows.setChecked(shouldShowShadows);

    connect(
        &lightingMenuWrapper.showShadows, &QAction::toggled, this,
        &MainWindow::OnShowShadowsToggled);

    lightingMenu.setTitle("Lighting");
    lightingMenu.setStatusTip("Toggle visualization aids");
    lightingMenu.addAction(&lightingMenuWrapper.showCascadeSplits);
    lightingMenu.addAction(&lightingMenuWrapper.showShadows);

    m_debuggingMenu.setTitle("Debugging");
    m_debuggingMenu.addMenu(&renderMenu);
    m_debuggingMenu.addMenu(&lightingMenu);

    menuBar()->addMenu(&m_debuggingMenu);
}

void MainWindow::SetupHelpMenu()
{
    m_helpMenuWrapper.aboutDialog.setParent(this);
    m_helpMenuWrapper.aboutDialog.setText("About");
    m_helpMenuWrapper.aboutDialog.setStatusTip("About D-Viz");

    connect(
        &m_helpMenuWrapper.aboutDialog, &QAction::triggered, this, &MainWindow::LaunchAboutDialog);

    m_helpMenu.setTitle("Help");
    m_helpMenu.addAction(&m_helpMenuWrapper.aboutDialog);

    menuBar()->addMenu(&m_helpMenu);
}

void MainWindow::SetDebuggingMenuState()
{
    auto& renderMenuWrapper = m_debuggingMenuWrapper.renderMenuWrapper;

    const auto& preferences = m_controller.GetPersistentSettings();

    renderMenuWrapper.origin.blockSignals(true);
    renderMenuWrapper.origin.setChecked(preferences.ShouldRenderOrigin());
    renderMenuWrapper.origin.blockSignals(false);

    renderMenuWrapper.grid.blockSignals(true);
    renderMenuWrapper.grid.setChecked(preferences.ShouldRenderGrid());
    renderMenuWrapper.grid.blockSignals(false);

    renderMenuWrapper.lightMarkers.blockSignals(true);
    renderMenuWrapper.lightMarkers.setChecked(preferences.ShouldRenderLightMarkers());
    renderMenuWrapper.lightMarkers.blockSignals(false);

    renderMenuWrapper.frustum.blockSignals(true);
    renderMenuWrapper.frustum.setChecked(preferences.ShouldRenderFrusta());
    renderMenuWrapper.frustum.blockSignals(false);
}

void MainWindow::OnFileMonitoringToggled(bool shouldEnable)
{
    auto& settings = m_controller.GetPersistentSettings();
    settings.MonitorFileSystem(shouldEnable);

    m_controller.MonitorFileSystem(shouldEnable);
}

void MainWindow::OnFileMenuNewScan()
{
    const auto locationList = QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation);

    const auto selectedDirectory = QFileDialog::getExistingDirectory(
        this, "Select a Directory to Visualize", locationList.front(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (selectedDirectory.isEmpty()) {
        return;
    }

    const auto fileSizeIndex = static_cast<std::size_t>(m_ui.minimumSizeComboBox->currentIndex());

    Settings::VisualizationParameters parameters;
    parameters.rootDirectory = selectedDirectory.toStdWString();
    parameters.onlyShowDirectories = m_showDirectoriesOnly;
    parameters.forceNewScan = true;
    parameters.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

    auto& savedParameters =
        m_controller.GetSessionSettings().SetVisualizationParameters(parameters);

    m_controller.ScanDrive(savedParameters);
}

bool MainWindow::AskUserToLimitFileSize(
    std::uintmax_t numberOfFilesScanned, Settings::VisualizationParameters parameters)
{
    using namespace Literals::Numeric::Binary;

    if (numberOfFilesScanned < 250'000 || parameters.minimumFileSize >= 1_MiB) {
        return false;
    }

    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::Yes);
    messageBox.setText("More than a quarter million files were scanned. "
                       "Would you like to limit the visualized files to those 1 MiB or larger in "
                       "order to reduce the load on the GPU and system memory?");

    const auto election = messageBox.exec();
    switch (election) {
        case QMessageBox::Yes: {
            parameters.minimumFileSize = 1_MiB;
            m_controller.GetSessionSettings().SetVisualizationParameters(std::move(parameters));
            SetFilePruningComboBoxValue(1_MiB);

            return true;
        }
        case QMessageBox::No: {
            return false;
        }
        default: {
            GSL_ASSUME(false);
        }
    }

    return false;
}

void MainWindow::OnFPSReadoutToggled(bool isEnabled)
{
    if (!isEnabled) {
        setWindowTitle("D-Viz [*]");
    }
}

void MainWindow::SwitchToBinaryPrefix(bool /*useBinary*/)
{
    // @todo Emit a signal that the breakdown dialog can hook up to.

    auto& menuWrapper = m_optionsMenuWrapper.fileSizeMenuWrapper;

    menuWrapper.binaryPrefix.blockSignals(true);
    menuWrapper.decimalPrefix.blockSignals(true);

    const ScopeExit onScopeExit = [&]() noexcept
    {
        m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(false);
        m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(false);
    };

    menuWrapper.binaryPrefix.setChecked(true);
    menuWrapper.decimalPrefix.setChecked(false);

    m_controller.GetSessionSettings().SetActiveNumericPrefix(Constants::SizePrefix::Binary);
    m_fileSizeOptions = GeneratePruningMenuEntries(Constants::SizePrefix::Binary);

    SetupFileSizePruningDropdown();

    const auto fileSizeIndex = static_cast<std::size_t>(m_ui.minimumSizeComboBox->currentIndex());
    if (fileSizeIndex < 1) {
        return;
    }

    auto& parameters = m_controller.GetSessionSettings().GetVisualizationParameters();
    parameters.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

    m_glCanvas->ReloadVisualization();
}

void MainWindow::SwitchToDecimalPrefix(bool /*useDecimal*/)
{
    // @todo Emit a signal that the breakdown dialog can hook up to.

    auto& menuWrapper = m_optionsMenuWrapper.fileSizeMenuWrapper;

    menuWrapper.binaryPrefix.blockSignals(true);
    menuWrapper.decimalPrefix.blockSignals(true);

    const ScopeExit onScopeExit = [&]() noexcept
    {
        m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(false);
        m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(false);
    };

    menuWrapper.binaryPrefix.setChecked(false);
    menuWrapper.decimalPrefix.setChecked(true);

    m_controller.GetSessionSettings().SetActiveNumericPrefix(Constants::SizePrefix::Decimal);
    m_fileSizeOptions = GeneratePruningMenuEntries(Constants::SizePrefix::Decimal);

    SetupFileSizePruningDropdown();

    const auto fileSizeIndex = static_cast<std::size_t>(m_ui.minimumSizeComboBox->currentIndex());
    if (fileSizeIndex < 1) {
        return;
    }

    auto& parameters = m_controller.GetSessionSettings().GetVisualizationParameters();
    parameters.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

    m_glCanvas->ReloadVisualization();
}

void MainWindow::OnNewSearchQuery()
{
    const auto searchQuery = m_ui.searchBox->text().toStdWString();

    const auto deselectionCallback = [&](auto& nodes) {
        m_glCanvas->RestoreHighlightedNodes(nodes);
    };

    const auto selectionCallback = [&](auto& nodes) { m_glCanvas->HighlightNodes(nodes); };

    const auto shouldSearchFiles = m_controller.GetSessionSettings().ShouldSearchFiles();
    const auto shouldSearchDirectories =
        m_controller.GetSessionSettings().ShouldSearchDirectories();

    const ScopedCursor waitCursor{ Qt::WaitCursor };

    m_controller.SearchTreeMap(
        searchQuery, deselectionCallback, selectionCallback, shouldSearchFiles,
        shouldSearchDirectories);
}

void MainWindow::OnSearchQueryTextChanged(const QString& text)
{
    m_ui.searchButton->setEnabled(text.size());
}

void MainWindow::OnApplyButtonPressed()
{
    if (!m_controller.HasModelBeenLoaded()) {
        return;
    }

    PruneTree();
    ApplyColorScheme();
}

void MainWindow::PruneTree()
{
    const auto pruneSizeIndex = static_cast<std::size_t>(m_ui.minimumSizeComboBox->currentIndex());
    const auto minimumSize = m_fileSizeOptions->at(pruneSizeIndex).first;

    Settings::VisualizationParameters parameters;
    parameters.rootDirectory = m_controller.GetRootPath().wstring();
    parameters.onlyShowDirectories = m_showDirectoriesOnly;
    parameters.forceNewScan = false;
    parameters.minimumFileSize = minimumSize;

    m_controller.GetSessionSettings().SetVisualizationParameters(parameters);

    if (!m_controller.GetRootPath().empty()) {
        m_glCanvas->ReloadVisualization();
    }

    if (m_breakdownDialog && m_breakdownDialog->isVisible()) {
        m_breakdownDialog->ReloadData();
    }
}

void MainWindow::ApplyColorScheme()
{
    const auto colorScheme = m_ui.colorSchemeComboBox->currentText().toStdWString();
    m_controller.GetNodePainter().SetColorScheme(colorScheme);

    m_glCanvas->ApplyColorScheme();
}

void MainWindow::OnFieldOfViewChange(int fieldOfView)
{
    m_glCanvas->SetFieldOfView(fieldOfView);
}

void MainWindow::OnDirectoryPruningChange(int state)
{
    m_showDirectoriesOnly = (state == Qt::Checked);
}

void MainWindow::OnShowBreakdownButtonPressed()
{
    if (!m_breakdownDialog) {
        m_breakdownDialog = std::make_unique<BreakdownDialog>(this);
    }

    m_breakdownDialog->show();
}

void MainWindow::OnRenderOriginToggled(bool shouldShow)
{
    m_glCanvas->ToggleAssetVisibility<Assets::Tag::OriginMarker>(shouldShow);
    m_controller.GetPersistentSettings().RenderOrigin(shouldShow);
}

void MainWindow::OnRenderGridToggled(bool shouldShow)
{
    m_glCanvas->ToggleAssetVisibility<Assets::Tag::Grid>(shouldShow);
    m_controller.GetPersistentSettings().RenderGrid(shouldShow);
}

void MainWindow::OnRenderLightMarkersToggled(bool shouldShow)
{
    m_glCanvas->ToggleAssetVisibility<Assets::Tag::LightMarker>(shouldShow);
    m_controller.GetPersistentSettings().RenderLightMarkers(shouldShow);
}

void MainWindow::OnRenderFrustaToggled(bool shouldShow)
{
    m_glCanvas->ToggleAssetVisibility<Assets::Tag::Frustum>(shouldShow);
    m_controller.GetPersistentSettings().RenderFrusta(shouldShow);
}

void MainWindow::OnShowShadowsToggled(bool shouldShow)
{
    m_controller.GetPersistentSettings().RenderShadows(shouldShow);
}

void MainWindow::OnShowCascadeSplitsToggled(bool shouldShow)
{
    m_controller.GetPersistentSettings().RenderCascadeSplits(shouldShow);
}

bool MainWindow::ShouldShowFrameTime() const
{
    return m_optionsMenuWrapper.toggleFrameTime.isChecked();
}

std::wstring MainWindow::GetSearchQuery() const
{
    return m_searchQuery;
}

Controller& MainWindow::GetController()
{
    return m_controller;
}

GLCanvas& MainWindow::GetCanvas()
{
    Expects(m_glCanvas);
    return *m_glCanvas;
}

Gamepad& MainWindow::GetGamepad()
{
    Expects(m_gamepad);
    return *m_gamepad;
}

void MainWindow::OnScanStarted()
{
    m_ui.showBreakdownButton->setEnabled(false);
}

void MainWindow::OnScanCompleted()
{
    ReloadVisualization();

    m_ui.showBreakdownButton->setEnabled(true);
}

std::shared_ptr<BaseTaskbarButton> MainWindow::GetTaskbarButton()
{
#if defined(Q_OS_WIN)
    return std::make_shared<WinTaskbarButton>(this);
#elif defined(Q_OS_LINUX)
    return std::make_shared<UnixTaskbarButton>(this);
#endif // Q_OS_LINUX
}

void MainWindow::LaunchAboutDialog()
{
    if (!m_aboutDialog) {
        m_aboutDialog = std::make_unique<AboutDialog>(this);
    }

    m_aboutDialog->show();
}

void MainWindow::SetFieldOfViewSlider(int fieldOfView)
{
    m_ui.fieldOfViewSlider->setValue(fieldOfView);
}

void MainWindow::SetCameraSpeedSpinner(double speed)
{
    m_ui.cameraSpeedSpinner->setValue(speed);
}

void MainWindow::SetFilePruningComboBoxValue(std::uintmax_t minimum)
{
    const auto match = std::find_if(
        std::begin(*m_fileSizeOptions),
        std::end(*m_fileSizeOptions), [minimum](const auto& sizeAndUnits) noexcept {
            return sizeAndUnits.first >= minimum;
        });

    if (match == std::end(*m_fileSizeOptions)) {
        return;
    }

    const int targetIndex =
        m_ui.minimumSizeComboBox->findData(static_cast<qulonglong>(match->first));

    if (targetIndex != -1) {
        m_ui.minimumSizeComboBox->setCurrentIndex(targetIndex);
    }
}

void MainWindow::SetStatusBarMessage(const std::wstring& message, int timeout /* = 0*/)
{
    auto* const statusBar = this->statusBar();
    if (!statusBar) {
        return;
    }

    statusBar->showMessage(QString::fromStdWString(message), timeout);
}

void MainWindow::ReloadVisualization()
{
    m_glCanvas->ReloadVisualization();
}

void MainWindow::DisplayErrorDialog(std::string_view message)
{
    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setStandardButtons(QMessageBox::Ok);
    messageBox.setDefaultButton(QMessageBox::Ok);
    messageBox.setText(message.data());

    messageBox.exec();
}

void MainWindow::SetWaitCursor()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void MainWindow::RestoreDefaultCursor()
{
    QApplication::restoreOverrideCursor();
}
