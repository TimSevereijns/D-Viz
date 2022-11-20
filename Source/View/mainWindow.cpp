#include "View/mainWindow.h"
#include "Settings/persistentSettings.h"
#include "Utilities/logging.h"
#include "Utilities/operatingSystem.h"
#include "Utilities/scopeExit.h"
#include "Utilities/scopedCursor.h"
#include "View/Viewport/glCanvas.h"
#include "constants.h"
#include "controller.h"
#include "literals.h"

#include <gsl/assert>
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
                // clang-format on

                return &binary;
            }
        }

        GSL_ASSUME(false);
    }

    /**
     * @brief Computes the coordinate needed to center the message box.
     *
     * @param[in] messageBox        The message box that is to be centered.
     * @param[in] mainWindow        The window within which the dialog should be centered.
     *
     * @return The top-left coordinate where the message box should be placed.
     */
    QPoint ComputeMessageBoxPosition(QMessageBox& messageBox, const MainWindow& mainWindow)
    {
        messageBox.show(); //< Force size computation.

        const auto windowPosition = mainWindow.pos();
        const auto windowSize = mainWindow.size();

        const auto x = windowPosition.x() + (windowSize.width() / 2) - (messageBox.width() / 2);
        const auto y = windowPosition.y() + (windowSize.height() / 2) - (messageBox.height() / 2);

        return { x, y };
    }

    /**
     * @brief Loads and applies a dark theme to the application.
     */
    void LoadAndApplyStyleSheet()
    {
        QFile styleSheetFile{ ":qdarkstyle/style.qss" };

        if (!styleSheetFile.exists()) {
            spdlog::get(Constants::Logging::DefaultLog)->error("Could not apply stylesheet.");
        } else {
            styleSheetFile.open(QFile::ReadOnly | QFile::Text);
            QTextStream stream{ &styleSheetFile };
            qApp->setStyleSheet(stream.readAll());
        }
    }

    void DisplayMessageDialog(
        const MainWindow& mainWindow, QMessageBox::Icon severity, std::string_view message)
    {
        QMessageBox messageBox;
        messageBox.setIcon(severity);
        messageBox.setStandardButtons(QMessageBox::Ok);
        messageBox.setDefaultButton(QMessageBox::Ok);
        messageBox.setText(message.data());

        const auto position = ComputeMessageBoxPosition(messageBox, mainWindow);
        messageBox.move(position);
        messageBox.exec();
    }

    SearchFlags BuildSearchFlags(const Settings::SessionSettings& settings)
    {
        SearchFlags flags{ 0 };

        if (settings.ShouldSearchFiles()) {
            flags |= SearchFlags::SearchFiles;
        }

        if (settings.ShouldSearchDirectories()) {
            flags |= SearchFlags::SearchDirectories;
        }

        if (settings.ShouldUseRegex()) {
            flags |= SearchFlags::UseRegex;
        }

        return flags;
    }
} // namespace

MainWindow::MainWindow(Controller& controller, QWidget* parent /* = nullptr */)
    : QMainWindow{ parent },
      m_controller{ controller },
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
    if (m_controller.GetPersistentSettings().ShouldUseDarkMode()) {
        LoadAndApplyStyleSheet();
    }

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
        m_ui.useRegex, &QCheckBox::stateChanged, &sessionSettings,
        &Settings::SessionSettings::UseRegexSearch);

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

    const auto& defaultScheme = QString::fromStdString(Constants::ColorScheme::Default);
    m_ui.colorSchemeComboBox->addItem(defaultScheme, defaultScheme);

    const auto& colorMap = m_controller.GetNodePainter().GetFileColorMap();
    for (const auto& extensionMap : colorMap) {
        const auto& categoryName = QString::fromStdString(extensionMap.first);
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
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    const auto gamepads = QGamepadManager::instance()->connectedGamepads();
    if (gamepads.isEmpty()) {
        return;
    }

    m_gamepad = std::make_unique<Gamepad>(*std::begin(gamepads), this);
    QGamepadManager::instance()->resetConfiguration(m_gamepad->deviceId());
#endif
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
    m_fileMenu.newScan.setText("New Scan...");
    m_fileMenu.newScan.setStatusTip("Start a new visualization.");
    m_fileMenu.newScan.setShortcuts(QKeySequence::New);

    connect(&m_fileMenu.newScan, &QAction::triggered, this, &MainWindow::OnFileMenuNewScan);

    m_fileMenu.cancelScan.setText("Cancel Scan");
    m_fileMenu.cancelScan.setStatusTip("Cancel active scan.");
    m_fileMenu.cancelScan.setEnabled(false);

    connect(&m_fileMenu.cancelScan, &QAction::triggered, this, &MainWindow::OnCancelScan);

    m_fileMenu.exit.setText("Exit");
    m_fileMenu.exit.setStatusTip("Exit the program.");
    m_fileMenu.exit.setShortcuts(QKeySequence::Quit);

    connect(&m_fileMenu.exit, &QAction::triggered, this, &MainWindow::OnClose);

    m_fileMenu.setTitle("File");
    m_fileMenu.addAction(&m_fileMenu.newScan);
    m_fileMenu.addAction(&m_fileMenu.cancelScan);
    m_fileMenu.addAction(&m_fileMenu.exit);

    menuBar()->addMenu(&m_fileMenu);
}

void MainWindow::SetupOptionsMenu()
{
    const auto& preferences = m_controller.GetPersistentSettings();

    m_optionsMenu.useDarkTheme.setText("Use Dark Theme");
    m_optionsMenu.useDarkTheme.setStatusTip("Toggles the use of a dark theme.");
    m_optionsMenu.useDarkTheme.setCheckable(true);
    m_optionsMenu.useDarkTheme.setChecked(preferences.ShouldUseDarkMode());

    connect(&m_optionsMenu.useDarkTheme, &QAction::toggled, this, &MainWindow::OnDarkThemeToggled);

    m_optionsMenu.enableFileSystemMonitoring.setEnabled(false);
    m_optionsMenu.enableFileSystemMonitoring.setText("Monitor File System");
    m_optionsMenu.enableFileSystemMonitoring.setStatusTip(
        "Monitors the file system for any changes");
    m_optionsMenu.enableFileSystemMonitoring.setCheckable(true);

    const auto isMonitoringEnabled = m_controller.GetPersistentSettings().ShouldMonitorFileSystem();
    m_optionsMenu.enableFileSystemMonitoring.setChecked(isMonitoringEnabled);

    connect(
        &m_optionsMenu.enableFileSystemMonitoring, &QAction::toggled, this,
        &MainWindow::OnFileMonitoringToggled);

    m_optionsMenu.setTitle("Options");
    m_optionsMenu.addAction(&m_optionsMenu.useDarkTheme);
    m_optionsMenu.addAction(&m_optionsMenu.enableFileSystemMonitoring);

    SetupFileSizeSubMenu();

    menuBar()->addMenu(&m_optionsMenu);
}

void MainWindow::SetupFileSizeSubMenu()
{
    auto& sizeMenu = m_optionsMenu.fileSizeMenu;

    sizeMenu.binaryPrefix.setText("Binary Prefix");
    sizeMenu.binaryPrefix.setStatusTip(
        "Use base-two units, such as kibibytes and mebibytes. This is the default on Windows.");
    sizeMenu.binaryPrefix.setCheckable(true);
    sizeMenu.binaryPrefix.setChecked(true);

    connect(&sizeMenu.binaryPrefix, &QAction::toggled, this, &MainWindow::SwitchToBinaryPrefix);

    sizeMenu.decimalPrefix.setText("Decimal Prefix");
    sizeMenu.decimalPrefix.setStatusTip("Use base-ten units, such as kilobytes and megabytes.");
    sizeMenu.decimalPrefix.setCheckable(true);
    sizeMenu.decimalPrefix.setChecked(false);

    connect(&sizeMenu.decimalPrefix, &QAction::toggled, this, &MainWindow::SwitchToDecimalPrefix);

    sizeMenu.setTitle("File Size Units");
    sizeMenu.addAction(&sizeMenu.binaryPrefix);
    sizeMenu.addAction(&sizeMenu.decimalPrefix);

    m_optionsMenu.addMenu(&m_optionsMenu.fileSizeMenu);
}

void MainWindow::SetupRenderSubMenu()
{
    auto& renderMenu = m_debuggingMenu.renderMenu;

    renderMenu.origin.setText("Origin");
    renderMenu.origin.setCheckable(true);
    renderMenu.origin.setChecked(false);

    connect(&renderMenu.origin, &QAction::toggled, this, &MainWindow::OnRenderOriginToggled);

    renderMenu.grid.setText("Grid");
    renderMenu.grid.setCheckable(true);
    renderMenu.grid.setChecked(false);

    connect(&renderMenu.grid, &QAction::toggled, this, &MainWindow::OnRenderGridToggled);

    renderMenu.lightMarkers.setText("Light Markers");
    renderMenu.lightMarkers.setCheckable(true);
    renderMenu.lightMarkers.setChecked(false);

    connect(
        &renderMenu.lightMarkers, &QAction::toggled, this,
        &MainWindow::OnRenderLightMarkersToggled);

    renderMenu.frustum.setText("Frustum");
    renderMenu.frustum.setCheckable(true);
    renderMenu.frustum.setChecked(false);

    connect(&renderMenu.frustum, &QAction::toggled, this, &MainWindow::OnRenderFrustaToggled);

    renderMenu.setTitle("Render Asset");
    renderMenu.setStatusTip("Toggle scene assets on or off");
    renderMenu.addAction(&renderMenu.origin);
    renderMenu.addAction(&renderMenu.grid);
    renderMenu.addAction(&renderMenu.lightMarkers);
    renderMenu.addAction(&renderMenu.frustum);
}

void MainWindow::SetupLightingSubMenu()
{
    auto& lightingMenu = m_debuggingMenu.lightingMenu;

    lightingMenu.showLightingOptions.setText("Show Lighting Options");
    lightingMenu.showLightingOptions.setStatusTip("Show additional lighting options.");
    lightingMenu.showLightingOptions.setCheckable(true);

    connect(
        &lightingMenu.showLightingOptions, &QAction::toggled, this,
        &MainWindow::OnShowLightingOptionsToggled);

    const auto shouldShowCascadeSplits =
        m_controller.GetPersistentSettings().ShouldRenderCascadeSplits();

    lightingMenu.showCascadeSplits.setText("Show Cascade Splits");
    lightingMenu.showCascadeSplits.setCheckable(true);
    lightingMenu.showCascadeSplits.setChecked(shouldShowCascadeSplits);

    connect(
        &lightingMenu.showCascadeSplits, &QAction::toggled, this,
        &MainWindow::OnShowCascadeSplitsToggled);

    const auto shouldShowShadows = m_controller.GetPersistentSettings().ShouldRenderShadows();

    lightingMenu.showShadows.setText("Show Shadows");
    lightingMenu.showShadows.setCheckable(true);
    lightingMenu.showShadows.setChecked(shouldShowShadows);

    connect(&lightingMenu.showShadows, &QAction::toggled, this, &MainWindow::OnShowShadowsToggled);

    lightingMenu.setTitle("Lighting");
    lightingMenu.setStatusTip("Toggle visualization aids");
    lightingMenu.addAction(&lightingMenu.showLightingOptions);
    lightingMenu.addAction(&lightingMenu.showCascadeSplits);
    lightingMenu.addAction(&lightingMenu.showShadows);
}

void MainWindow::SetupDebuggingMenu()
{
    SetupRenderSubMenu();
    SetupLightingSubMenu();

    m_debuggingMenu.setTitle("Debugging");
    m_debuggingMenu.addMenu(&m_debuggingMenu.renderMenu);
    m_debuggingMenu.addMenu(&m_debuggingMenu.lightingMenu);

    m_debuggingMenu.openLogFile.setText("Open Log File");

    connect(&m_debuggingMenu.openLogFile, &QAction::triggered, this, &MainWindow::OnOpenLogFile);

    m_debuggingMenu.addAction(&m_debuggingMenu.openLogFile);

    m_debuggingMenu.toggleFrameTime.setText("Show Frame Time");
    m_debuggingMenu.toggleFrameTime.setStatusTip("Toggle frame-time readout in titlebar.");
    m_debuggingMenu.toggleFrameTime.setCheckable(true);

    connect(
        &m_debuggingMenu.toggleFrameTime, &QAction::toggled, this,
        &MainWindow::OnFpsReadoutToggled);

    m_debuggingMenu.addAction(&m_debuggingMenu.toggleFrameTime);

    menuBar()->addMenu(&m_debuggingMenu);
}

void MainWindow::SetupHelpMenu()
{
    m_helpMenu.aboutDialog.setParent(this);
    m_helpMenu.aboutDialog.setText("About...");
    m_helpMenu.aboutDialog.setStatusTip("About D-Viz");

    connect(&m_helpMenu.aboutDialog, &QAction::triggered, this, &MainWindow::LaunchAboutDialog);

    m_helpMenu.setTitle("Help");
    m_helpMenu.addAction(&m_helpMenu.aboutDialog);

    menuBar()->addMenu(&m_helpMenu);
}

void MainWindow::SetDebuggingMenuState()
{
    auto& renderMenu = m_debuggingMenu.renderMenu;

    const auto& preferences = m_controller.GetPersistentSettings();

    renderMenu.origin.blockSignals(true);
    renderMenu.origin.setChecked(preferences.ShouldRenderOrigin());
    renderMenu.origin.blockSignals(false);

    renderMenu.grid.blockSignals(true);
    renderMenu.grid.setChecked(preferences.ShouldRenderGrid());
    renderMenu.grid.blockSignals(false);

    renderMenu.lightMarkers.blockSignals(true);
    renderMenu.lightMarkers.setChecked(preferences.ShouldRenderLightMarkers());
    renderMenu.lightMarkers.blockSignals(false);

    renderMenu.frustum.blockSignals(true);
    renderMenu.frustum.setChecked(preferences.ShouldRenderFrusta());
    renderMenu.frustum.blockSignals(false);

    HideLightingOptions();
}

void MainWindow::ShowLightingOptions()
{
    m_ui.ambientCoefficientSpinnerLabel->show();
    m_ui.ambientCoefficientSpinner->show();

    m_ui.attentuationMultiplierSpinnerLabel->show();
    m_ui.attenuationSpinner->show();

    m_ui.attachLightToCameraCheckBox->show();
    m_ui.lightingDivider->show();
}

void MainWindow::HideLightingOptions()
{
    m_ui.ambientCoefficientSpinnerLabel->hide();
    m_ui.ambientCoefficientSpinner->hide();

    m_ui.attentuationMultiplierSpinnerLabel->hide();
    m_ui.attenuationSpinner->hide();

    m_ui.attachLightToCameraCheckBox->hide();
    m_ui.lightingDivider->hide();
}

void MainWindow::OnFileMonitoringToggled(bool shouldEnable)
{
    auto& settings = m_controller.GetPersistentSettings();
    settings.MonitorFileSystem(shouldEnable);

    m_controller.MonitorFileSystem(shouldEnable);
}

void MainWindow::OnClose()
{
    m_controller.StopScanning();
    close();
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

    Settings::VisualizationOptions options;
    options.rootDirectory = selectedDirectory.toStdString();
    options.onlyShowDirectories = m_showDirectoriesOnly;
    options.forceNewScan = true;
    options.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

    const auto& savedOptions =
        m_controller.GetSessionSettings().SetVisualizationOptions(std::move(options));

    m_controller.ScanDrive(savedOptions);
}

bool MainWindow::AskUserToLimitFileSize(std::uintmax_t numberOfFilesScanned)
{
    using namespace Literals::Numeric::Binary;

    auto& options = m_controller.GetSessionSettings().GetVisualizationOptions();
    if (numberOfFilesScanned < 250'000 || options.minimumFileSize >= 1_MiB) {
        return false;
    }

    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Warning);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::Yes);
    messageBox.setText(
        "More than a quarter million files were scanned. Would you like to exclude "
        "files smaller than 1 MiB to ease GPU load?");

    const auto position = ComputeMessageBoxPosition(messageBox, *this);
    messageBox.move(position);

    const auto election = messageBox.exec();
    if (election == QMessageBox::Yes) {
        options.minimumFileSize = 1_MiB;
        SetFilePruningComboBoxValue(options.minimumFileSize);

        return true;
    }

    return false;
}

bool MainWindow::AskUserToConfirmDeletion(const std::filesystem::path& filePath)
{
    QMessageBox messageBox;
    messageBox.setIcon(QMessageBox::Question);
    messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    messageBox.setDefaultButton(QMessageBox::Yes);

    const auto fileName = QString::fromStdString(filePath.filename().string());
    messageBox.setText("Are you sure you want to delete the following file? \n\n" + fileName);

    const auto position = ComputeMessageBoxPosition(messageBox, *this);
    messageBox.move(position);

    const auto election = messageBox.exec();
    return election == QMessageBox::Yes;
}

void MainWindow::OnFpsReadoutToggled(bool isEnabled)
{
    if (!isEnabled) {
        setWindowTitle("D-Viz [*]");
    }
}

void MainWindow::OnShowLightingOptionsToggled(bool isEnabled)
{
    if (isEnabled) {
        ShowLightingOptions();
    } else {
        HideLightingOptions();
    }
}

void MainWindow::OnDarkThemeToggled(bool isEnabled)
{
    auto& settings = m_controller.GetPersistentSettings();

    settings.UseDarkMode(isEnabled);
    settings.SaveAllPreferencesToDisk();

    DisplayInfoDialog("Please restart D-Viz to complete theme switch.");
}

void MainWindow::SwitchToBinaryPrefix(bool /*useBinary*/)
{
    // @todo Emit a signal that the breakdown dialog can hook up to.

    auto& menuWrapper = m_optionsMenu.fileSizeMenu;

    menuWrapper.binaryPrefix.blockSignals(true);
    menuWrapper.decimalPrefix.blockSignals(true);

    const ScopeExit unblockSignals = [&]() noexcept
    {
        m_optionsMenu.fileSizeMenu.decimalPrefix.blockSignals(false);
        m_optionsMenu.fileSizeMenu.binaryPrefix.blockSignals(false);
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

    auto& options = m_controller.GetSessionSettings().GetVisualizationOptions();
    options.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

    m_glCanvas->ReloadVisualization();
}

void MainWindow::SwitchToDecimalPrefix(bool /*useDecimal*/)
{
    // @todo Emit a signal that the breakdown dialog can hook up to.

    auto& menuWrapper = m_optionsMenu.fileSizeMenu;

    menuWrapper.binaryPrefix.blockSignals(true);
    menuWrapper.decimalPrefix.blockSignals(true);

    const ScopeExit unblockSignals = [&]() noexcept
    {
        m_optionsMenu.fileSizeMenu.decimalPrefix.blockSignals(false);
        m_optionsMenu.fileSizeMenu.binaryPrefix.blockSignals(false);
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

    auto& options = m_controller.GetSessionSettings().GetVisualizationOptions();
    options.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

    m_glCanvas->ReloadVisualization();
}

void MainWindow::OnNewSearchQuery()
{
    const auto searchQuery = m_ui.searchBox->text().toStdString();

    const auto deselectionCallback = [&](auto& nodes) {
        m_glCanvas->RestoreHighlightedNodes(nodes);
    };

    const auto selectionCallback = [&](auto& nodes) { m_glCanvas->HighlightNodes(nodes); };

    const ScopedCursor waitCursor{ Qt::WaitCursor };

    const auto flags = BuildSearchFlags(m_controller.GetSessionSettings());
    m_controller.SearchTreeMap(searchQuery, deselectionCallback, selectionCallback, flags);
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

    Settings::VisualizationOptions options;
    options.rootDirectory = m_controller.GetRootPath().string();
    options.onlyShowDirectories = m_showDirectoriesOnly;
    options.forceNewScan = false;
    options.minimumFileSize = minimumSize;

    m_controller.GetSessionSettings().SetVisualizationOptions(options);

    if (!m_controller.GetRootPath().empty()) {
        m_glCanvas->ReloadVisualization();
    }

    if (m_breakdownDialog && m_breakdownDialog->isVisible()) {
        m_breakdownDialog->ReloadData();
    }
}

void MainWindow::ApplyColorScheme()
{
    const auto colorScheme = m_ui.colorSchemeComboBox->currentText().toStdString();
    m_controller.GetNodePainter().SetActiveColorScheme(colorScheme);

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
    } else {
        m_breakdownDialog->ReloadData();
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

void MainWindow::OnOpenLogFile()
{
    OS::OpenFile(Logging::GetDefaultLogPath());
}

void MainWindow::OnCancelScan()
{
    m_controller.StopScanning();
}

bool MainWindow::ShouldShowFrameTime() const
{
    return m_debuggingMenu.toggleFrameTime.isChecked();
}

std::string MainWindow::GetSearchQuery() const
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
    m_fileMenu.cancelScan.setEnabled(true);
}

void MainWindow::OnScanCompleted()
{
    ReloadVisualization();

    m_ui.showBreakdownButton->setEnabled(true);
    m_fileMenu.cancelScan.setEnabled(false);
    m_optionsMenu.enableFileSystemMonitoring.setEnabled(true);
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

void MainWindow::SetStatusBarMessage(const std::string& message, int timeout /* = 0*/)
{
    auto* const statusBar = this->statusBar();
    if (!statusBar) {
        return;
    }

    statusBar->showMessage(QString::fromStdString(message), timeout);
}

void MainWindow::ReloadVisualization()
{
    m_glCanvas->ReloadVisualization();
}

void MainWindow::DisplayInfoDialog(std::string_view message)
{
    DisplayMessageDialog(*this, QMessageBox::Information, message);
}

void MainWindow::DisplayErrorDialog(std::string_view message)
{
    DisplayMessageDialog(*this, QMessageBox::Warning, message);
}

void MainWindow::SetWaitCursor()
{
    QApplication::setOverrideCursor(Qt::WaitCursor);
}

void MainWindow::RestoreDefaultCursor()
{
    QApplication::restoreOverrideCursor();
}
