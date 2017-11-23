#include "mainWindow.h"

#include "constants.h"
#include "globals.h"
#include "literals.h"

#include "DataStructs/scanningProgress.hpp"
#include "Settings/settings.h"
#include "Settings/settingsManager.h"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Utilities/utilities.hpp"
#include "Viewport/glCanvas.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

namespace
{
   /**
    * @brief Helper function to be called once scanning completes.
    *
    * @param[in] progress           The final results from the scan.
    */
   void LogScanCompletion(const ScanningProgress& progress)
   {
      const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);

      log->info(
         fmt::format("Scanned: {} directories and {} files, representing {} bytes",
         progress.directoriesScanned.load(),
         progress.filesScanned.load(),
         progress.bytesProcessed.load()));

      log->flush();
   }

   /**
    * @brief Switches to a different set of entries for the file pruning drop-down menu.
    *
    * @param[in] prefix             The desired unit prefix.
    *
    * @returns A pointer to a const static vector containing the menu values.
    */
   const std::vector<std::pair<std::uintmax_t, QString>>* GeneratePruningMenuEntries(
      Constants::FileSize::Prefix prefix)
   {
      switch (prefix)
      {
         case Constants::FileSize::Prefix::DECIMAL:
         {
            using namespace Literals::Numeric::Decimal;

            const static auto decimal = std::vector<std::pair<std::uintmax_t, QString>>
            {
               { 0u,     "Show All" },
               { 1_KB,   "< 1 KB"   },
               { 1_MB,   "< 1 MB"   },
               { 10_MB,  "< 10 MB"  },
               { 100_MB, "< 100 MB" },
               { 250_MB, "< 250 MB" },
               { 500_MB, "< 500 MB" },
               { 1_GB,   "< 1 GB"   },
               { 5_GB,   "< 5 GB"   },
               { 10_GB,  "< 10 GB"  }
            };

            return &decimal;
         }
         case Constants::FileSize::Prefix::BINARY:
         {
            using namespace Literals::Numeric::Binary;

            const static auto binary = std::vector<std::pair<std::uintmax_t, QString>>
            {
               { 0u,     "Show All" },
               { 1_KiB,   "< 1 KiB"   },
               { 1_MiB,   "< 1 MiB"   },
               { 10_MiB,  "< 10 MiB"  },
               { 100_MiB, "< 100 MiB" },
               { 250_MiB, "< 250 MiB" },
               { 500_MiB, "< 500 MiB" },
               { 1_GiB,   "< 1 GiB"   },
               { 5_GiB,   "< 5 GiB"   },
               { 10_GiB,  "< 10 GiB"  }
            };

            return &binary;
         }
         default:
         {
            assert(!"Type not supported.");
         }

         return nullptr;
      }
   }

   /**
    * @returns The full path to the JSON file that contains the color mapping.
    */
   auto GetColorJsonPath()
   {
      return std::experimental::filesystem::current_path().append(L"colors.json");
   }

   /**
    * @returns The full path to the JSON file that contains the user preferences.
    */
   auto GetPreferencesJsonPath()
   {
      return std::experimental::filesystem::current_path().append(L"preferences.json");
   }
}

MainWindow::MainWindow(
   Controller& controller,
   QWidget* parent /* = nullptr */)
   :
   QMainWindow{ parent },
   m_controller{ controller },
   m_settingsManager{ GetColorJsonPath(), GetPreferencesJsonPath() },
   m_fileSizeOptions{ GeneratePruningMenuEntries(Constants::FileSize::Prefix::BINARY) }
{
   m_ui.setupUi(this);

   m_glCanvas = std::make_unique<GLCanvas>(controller, this);
   m_ui.canvasLayout->addWidget(m_glCanvas.get());

   SetupMenus();
   SetupGamepad();
   SetupSidebar();

   SetDebuggingMenuState();
}

void MainWindow::SetupSidebar()
{
   SetupColorSchemeDropdown();
   SetupFileSizePruningDropdown();

   connect(m_ui.directoriesOnlyCheckBox, &QCheckBox::stateChanged,
      this, &MainWindow::OnDirectoryPruningChange);

   connect(m_ui.directoryGradientCheckBox, &QCheckBox::stateChanged,
      this, &MainWindow::OnGradientUseChange);

   connect(m_ui.pruneTreeButton, &QPushButton::clicked,
      this, &MainWindow::PruneTree);

   connect(m_ui.fieldOfViewSlider, &QSlider::valueChanged,
      this, &MainWindow::OnFieldOfViewChange);

   connect(m_ui.searchBox, &QLineEdit::returnPressed,
      this, &MainWindow::OnNewSearchQuery);

   connect(m_ui.searchButton, &QPushButton::clicked,
      this, &MainWindow::OnNewSearchQuery);

   connect(m_ui.showBreakdownButton, &QPushButton::clicked,
      this, &MainWindow::OnShowBreakdownButtonPressed);

   connect(m_ui.colorSchemeComboBox, &QComboBox::currentTextChanged,
      this, &MainWindow::OnColorSchemeChanged);

   connect(m_ui.searchDirectoriesCheckBox, &QCheckBox::stateChanged,
      &m_settingsManager, &Settings::Manager::OnShouldSearchDirectoriesChanged);

   connect(m_ui.searchFilesCheckBox, &QCheckBox::stateChanged,
      &m_settingsManager, &Settings::Manager::OnShouldSearchFilesChanged);

   connect(m_ui.cameraSpeedSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      &m_settingsManager, &Settings::Manager::OnCameraSpeedChanged);

   connect(m_ui.mouseSensitivitySpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      &m_settingsManager, &Settings::Manager::OnMouseSensitivityChanged);

   connect(m_ui.ambientCoefficientSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      &m_settingsManager, &Settings::Manager::OnAmbientLightCoefficientChanged);

   connect(m_ui.attenuationSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      &m_settingsManager, &Settings::Manager::OnLightAttenuationChanged);

   connect(m_ui.shininesSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      &m_settingsManager, &Settings::Manager::OnMaterialShininessChanged);

   connect(m_ui.attachLightToCameraCheckBox,
      static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
      &m_settingsManager, &Settings::Manager::OnAttachLightToCameraStateChanged);
}

void MainWindow::SetupColorSchemeDropdown()
{
   m_ui.colorSchemeComboBox->clear();

   const auto& defaultScheme = QString::fromStdWString(L"Default");
   m_ui.colorSchemeComboBox->addItem(defaultScheme, defaultScheme);

   const auto& colorMap = m_settingsManager.GetFileColorMap();
   for (const auto& extensionMap : colorMap)
   {
      const auto& categoryName = QString::fromStdWString(extensionMap.first);
      m_ui.colorSchemeComboBox->addItem(categoryName, categoryName);
   }
}

void MainWindow::SetupFileSizePruningDropdown()
{
   const auto previousIndex = m_ui.pruneSizeComboBox->currentIndex();

   m_ui.pruneSizeComboBox->clear();

   for (const auto& fileSizeAndUnits : *m_fileSizeOptions)
   {
      m_ui.pruneSizeComboBox->addItem(fileSizeAndUnits.second,
         static_cast<qulonglong>(fileSizeAndUnits.first));
   }

   m_ui.pruneSizeComboBox->setCurrentIndex(previousIndex == -1 ? 0 : previousIndex);

   statusBar()->clearMessage();
}

void MainWindow::SetupGamepad()
{
   const auto gamepads = QGamepadManager::instance()->connectedGamepads();
   if (gamepads.isEmpty())
   {
      return;
   }

   m_gamepad = std::make_unique<Gamepad>(*gamepads.begin(), this);
   QGamepadManager::instance()->resetConfiguration(m_gamepad->deviceId());
}

std::wstring MainWindow::GetDirectoryToVisualize() const
{
   return m_rootPath.wstring();
}

void MainWindow::SetupMenus()
{
   SetupFileMenu();
   SetupOptionsMenu();

   // @todo Guard this menu with a boolean of some sort:
   SetupDebuggingMenu();

   SetupHelpMenu();
}

void MainWindow::SetupFileMenu()
{
   m_fileMenuWrapper.newScan.setText("New Scan...");
   m_fileMenuWrapper.newScan.setStatusTip("Start a new visualization.");
   m_fileMenuWrapper.newScan.setShortcuts(QKeySequence::New);

   connect(&m_fileMenuWrapper.newScan, &QAction::triggered,
      this, &MainWindow::OnFileMenuNewScan);

   m_fileMenuWrapper.exit.setText("Exit");
   m_fileMenuWrapper.exit.setStatusTip("Exit the program.");
   m_fileMenuWrapper.exit.setShortcuts(QKeySequence::Quit);

   connect(&m_fileMenuWrapper.exit, &QAction::triggered,
      this, &MainWindow::close);

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

   connect(&m_optionsMenuWrapper.toggleFrameTime, &QAction::toggled,
      this, &MainWindow::OnFPSReadoutToggled);

   m_optionsMenu.setTitle("Options");
   m_optionsMenu.addAction(&m_optionsMenuWrapper.toggleFrameTime);

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

   connect(&subMenuWrapper.binaryPrefix, &QAction::toggled,
      this, &MainWindow::SwitchToBinaryPrefix);

   subMenuWrapper.decimalPrefix.setText("Decimal Prefix");
   subMenuWrapper.decimalPrefix.setStatusTip(
      "Use base-ten units, such as kilobytes and megabytes.");
   subMenuWrapper.decimalPrefix.setCheckable(true);
   subMenuWrapper.decimalPrefix.setChecked(false);

   connect(&subMenuWrapper.decimalPrefix, &QAction::toggled,
      this, &MainWindow::SwitchToDecimalPrefix);

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
   renderMenuWrapper.origin.setChecked(true);

   connect(&renderMenuWrapper.origin, &QAction::toggled,
      this, &MainWindow::OnRenderOriginToggled);

   renderMenuWrapper.grid.setText("Grid");
   renderMenuWrapper.grid.setCheckable(true);
   renderMenuWrapper.grid.setChecked(true);

   connect(&renderMenuWrapper.grid, &QAction::toggled,
      this, &MainWindow::OnRenderGridToggled);

   renderMenuWrapper.lightMarkers.setText("Light Markers");
   renderMenuWrapper.lightMarkers.setCheckable(true);
   renderMenuWrapper.lightMarkers.setChecked(true);

   connect(&renderMenuWrapper.lightMarkers, &QAction::toggled,
      this, &MainWindow::OnRenderLightMarkersToggled);

   renderMenu.setTitle("Render");
   renderMenu.setStatusTip("Toggle visual debugging aides.");
   renderMenu.addAction(&renderMenuWrapper.origin);
   renderMenu.addAction(&renderMenuWrapper.grid);
   renderMenu.addAction(&renderMenuWrapper.lightMarkers);

   m_debuggingMenu.setTitle("Debugging");
   m_debuggingMenu.addMenu(&renderMenu);

   menuBar()->addMenu(&m_debuggingMenu);
}

void MainWindow::SetupHelpMenu()
{
   m_helpMenuWrapper.aboutDialog.setParent(this);
   m_helpMenuWrapper.aboutDialog.setText("About");
   m_helpMenuWrapper.aboutDialog.setStatusTip("About D-Viz");

   connect(&m_helpMenuWrapper.aboutDialog, &QAction::triggered,
      this, &MainWindow::LaunchAboutDialog);

   m_helpMenu.setTitle("Help");
   m_helpMenu.addAction(&m_helpMenuWrapper.aboutDialog);

   menuBar()->addMenu(&m_helpMenu);
}

void MainWindow::SetDebuggingMenuState()
{
   auto& renderMenuWrapper = m_debuggingMenuWrapper.renderMenuWrapper;

   const auto& preferences = m_settingsManager.GetPreferenceMap();

   const auto shouldShowOrigin = preferences.GetValueOrDefault(L"showOriginMarker", true);
   renderMenuWrapper.origin.blockSignals(true);
   renderMenuWrapper.origin.setChecked(shouldShowOrigin);
   renderMenuWrapper.origin.blockSignals(false);

   const auto shouldShowGrid = preferences.GetValueOrDefault(L"showGrid", true);
   renderMenuWrapper.grid.blockSignals(true);
   renderMenuWrapper.grid.setChecked(shouldShowGrid);
   renderMenuWrapper.grid.blockSignals(false);

   const auto shouldShowLightMarkers = preferences.GetValueOrDefault(L"showLightMarker", true);
   renderMenuWrapper.lightMarkers.blockSignals(true);
   renderMenuWrapper.lightMarkers.setChecked(shouldShowLightMarkers);
   renderMenuWrapper.lightMarkers.blockSignals(false);
}

void MainWindow::OnFileMenuNewScan()
{
   const auto selectedDirectory = QFileDialog::getExistingDirectory(
      this,
      "Select a Directory to Visualize",
      "/home",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   if (selectedDirectory.isEmpty())
   {
      assert(false);
      return;
   }

   m_rootPath = selectedDirectory.toStdWString();

   const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
   log->info(fmt::format("Started a new scan at: \"{}\"", m_rootPath.string()));

   const auto fileSizeIndex = m_ui.pruneSizeComboBox->currentIndex();

   Settings::VisualizationParameters parameters;
   parameters.rootDirectory = m_rootPath.wstring();
   parameters.onlyShowDirectories = m_showDirectoriesOnly;
   parameters.forceNewScan = true;
   parameters.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

   auto& savedParameters = m_settingsManager.SetVisualizationParameters(std::move(parameters));
   ScanDrive(savedParameters);
}

void MainWindow::ScanDrive(Settings::VisualizationParameters& parameters)
{
   m_occupiedDiskSpace = OperatingSystemSpecific::GetUsedDiskSpace(parameters.rootDirectory);
   assert(m_occupiedDiskSpace > 0);

   const auto progressHandler = [&] (const ScanningProgress& progress)
   {
      ComputeProgress(progress);
   };

   const auto completionHandler = [&, parameters] (
      const ScanningProgress& progress,
      const std::shared_ptr<Tree<VizFile>>& scanningResults) mutable
   {
      ComputeProgress(progress);
      LogScanCompletion(progress);

      m_controller.SaveScanResults(progress);

      QCursor previousCursor = cursor();
      setCursor(Qt::WaitCursor);
      ON_SCOPE_EXIT{ setCursor(previousCursor); };
      QApplication::processEvents();

      AskUserToLimitFileSize(progress.filesScanned.load(), parameters);

      m_controller.ParseResults(scanningResults);
      m_controller.UpdateBoundingBoxes();

      m_glCanvas->ReloadVisualization();

      m_controller.AllowUserInteractionWithModel(true);
      m_ui.showBreakdownButton->setEnabled(true);
   };

   m_controller.ResetVisualization();

   m_controller.AllowUserInteractionWithModel(false);
   m_ui.showBreakdownButton->setEnabled(false);

   DriveScanningParameters scanningParameters
   {
      parameters.rootDirectory,
      progressHandler,
      completionHandler
   };

   m_scanner.StartScanning(std::move(scanningParameters));
}

bool MainWindow::AskUserToLimitFileSize(
   std::uintmax_t numberOfFilesScanned,
   Settings::VisualizationParameters& parameters)
{
   using namespace Literals::Numeric::Binary;

   if (numberOfFilesScanned < 250'000 || parameters.minimumFileSize >= 1_MiB)
   {
      return false;
   }

   QMessageBox messageBox;
   messageBox.setIcon(QMessageBox::Warning);
   messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   messageBox.setDefaultButton(QMessageBox::Yes);
   messageBox.setText(
      "More than a quarter million files were scanned. "
      "Would you like to limit the visualized files to those 1 MiB or larger in "
      "order to reduce the load on the GPU and system memory?");

   const auto election = messageBox.exec();
   switch (election)
   {
      case QMessageBox::Yes:
      {
         parameters.minimumFileSize = 1_MiB;
         m_settingsManager.SetVisualizationParameters(parameters);
         SetFilePruningComboBoxValue(1_MiB);

         return true;
      }
      case QMessageBox::No:
      {
         return false;
      }
      default:
      {
         assert(false);
      }
   }

   return false;
}

void MainWindow::OnFPSReadoutToggled(bool isEnabled)
{
   if (!isEnabled)
   {
      setWindowTitle("D-Viz [*]");
   }
}

void MainWindow::SwitchToBinaryPrefix(bool /*useBinary*/)
{
   // @todo Emit a signal that the breakdown dialog can hook up to.

   auto& menuWrapper = m_optionsMenuWrapper.fileSizeMenuWrapper;

   menuWrapper.binaryPrefix.blockSignals(true);
   menuWrapper.decimalPrefix.blockSignals(true);

   ON_SCOPE_EXIT
   {
      m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(false);
      m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(false);
   };

   menuWrapper.binaryPrefix.setChecked(true);
   menuWrapper.decimalPrefix.setChecked(false);

   Globals::ActivePrefix = Constants::FileSize::Prefix::BINARY;
   m_fileSizeOptions = GeneratePruningMenuEntries(Globals::ActivePrefix);

   SetupFileSizePruningDropdown();

   const auto fileSizeIndex = m_ui.pruneSizeComboBox->currentIndex();
   if (fileSizeIndex < 1)
   {
      return;
   }

   auto& parameters = m_settingsManager.GetVisualizationParameters();
   parameters.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

   m_glCanvas->ReloadVisualization();
}

void MainWindow::SwitchToDecimalPrefix(bool /*useDecimal*/)
{
   // @todo Emit a signal that the breakdown dialog can hook up to.

   auto& menuWrapper = m_optionsMenuWrapper.fileSizeMenuWrapper;

   menuWrapper.binaryPrefix.blockSignals(true);
   menuWrapper.decimalPrefix.blockSignals(true);

   ON_SCOPE_EXIT
   {
      m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(false);
      m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(false);
   };

   menuWrapper.binaryPrefix.setChecked(false);
   menuWrapper.decimalPrefix.setChecked(true);

   Globals::ActivePrefix = Constants::FileSize::Prefix::DECIMAL;
   m_fileSizeOptions = GeneratePruningMenuEntries(Globals::ActivePrefix);

   SetupFileSizePruningDropdown();

   const auto fileSizeIndex = m_ui.pruneSizeComboBox->currentIndex();
   if (fileSizeIndex < 1)
   {
      return;
   }

   auto& parameters = m_settingsManager.GetVisualizationParameters();
   parameters.minimumFileSize = m_fileSizeOptions->at(fileSizeIndex).first;

   m_glCanvas->ReloadVisualization();
}

void MainWindow::OnNewSearchQuery()
{
   const auto searchQuery = m_ui.searchBox->text().toStdWString();

   const auto deselectionCallback = [&] (auto& nodes)
   {
      m_glCanvas->RestoreHighlightedNodes(nodes);
   };

   const auto selectionCallback = [&] (auto& nodes)
   {
      m_glCanvas->HighlightNodes(nodes);
   };

   const auto shouldSearchFiles = m_ui.searchFilesCheckBox->isChecked();
   const auto shouldSearchDirectories = m_ui.searchDirectoriesCheckBox->isChecked();

   m_controller.SearchTreeMap(
      searchQuery,
      deselectionCallback,
      selectionCallback,
      shouldSearchFiles,
      shouldSearchDirectories);
}

void MainWindow::PruneTree()
{
   const auto pruneSizeIndex = m_ui.pruneSizeComboBox->currentIndex();

   Settings::VisualizationParameters parameters;
   parameters.rootDirectory = m_rootPath.wstring();
   parameters.onlyShowDirectories = m_showDirectoriesOnly;
   parameters.useDirectoryGradient = m_useDirectoryGradient;
   parameters.forceNewScan = false;
   parameters.minimumFileSize = m_fileSizeOptions->at(pruneSizeIndex).first;

   m_settingsManager.SetVisualizationParameters(parameters);

   if (!m_rootPath.empty())
   {
      m_glCanvas->ReloadVisualization();
   }
}

void MainWindow::OnFieldOfViewChange(int fieldOfView)
{
   m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
}

void MainWindow::OnDirectoryPruningChange(int state)
{
   m_showDirectoriesOnly = (state == Qt::Checked);
}

void MainWindow::OnGradientUseChange(int state)
{
   m_useDirectoryGradient = (state == Qt::Checked);
}

void MainWindow::OnShowBreakdownButtonPressed()
{
   if (!m_breakdownDialog)
   {
      m_breakdownDialog = std::make_unique<BreakdownDialog>(this);
   }

   m_breakdownDialog->show();
}

void MainWindow::OnRenderOriginToggled(bool isEnabled)
{
   m_glCanvas->ToggleAssetVisibility<Asset::Tag::OriginMarker>(isEnabled);
}

void MainWindow::OnRenderGridToggled(bool isEnabled)
{
   m_glCanvas->ToggleAssetVisibility<Asset::Tag::Grid>(isEnabled);
}

void MainWindow::OnRenderLightMarkersToggled(bool isEnabled)
{
   m_glCanvas->ToggleAssetVisibility<Asset::Tag::LightMarker>(isEnabled);
}

void MainWindow::OnColorSchemeChanged(const QString& scheme)
{
   m_settingsManager.SetColorScheme(scheme.toStdWString());
   m_glCanvas->ApplyColorScheme();
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
   assert(m_glCanvas);
   return *m_glCanvas;
}

Gamepad& MainWindow::GetGamepad()
{
   assert(m_gamepad);
   return *m_gamepad;
}

void MainWindow::LaunchAboutDialog()
{
   if (!m_aboutDialog)
   {
      m_aboutDialog = std::make_unique<AboutDialog>(this);
   }

   m_aboutDialog->show();
}

void MainWindow::ComputeProgress(const ScanningProgress& progress)
{
   assert(m_occupiedDiskSpace > 0);

   const auto filesScanned = progress.filesScanned.load();
   const auto bytesProcessed = progress.bytesProcessed.load();

   const auto doesPathRepresentEntireDrive{ m_rootPath.string() == m_rootPath.root_path() };
   if (doesPathRepresentEntireDrive)
   {
      // @todo Progress can report as being more than 100% due to an issue in the implementation of
      // std::experimental::filesystem. This issue causes the API to report junctions as regular old
      // directories instead of some type of link. This means that instead of being skipped, any
      // junctions encountered during scanning will be explored and counted against the byte total.

      const auto percentComplete =
         100 * (static_cast<double>(bytesProcessed) / static_cast<double>(m_occupiedDiskSpace));

      const auto message = fmt::format(L"Files Scanned: {}  |  {:03.2f}% Complete",
         Utilities::StringifyWithDigitSeparators(filesScanned),
         percentComplete);

      SetStatusBarMessage(message.c_str());
   }
   else
   {
      const auto [size, units] = Controller::ConvertFileSizeToAppropriateUnits(bytesProcessed);

      const auto message = fmt::format(L"Files Scanned: {}  |  {:03.2f} {} and counting...",
         Utilities::StringifyWithDigitSeparators(filesScanned), size, units);

      SetStatusBarMessage(message.c_str());
   }
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
      std::end(*m_fileSizeOptions),
      [minimum] (const auto& sizeAndUnits) noexcept
   {
      return sizeAndUnits.first >= minimum;
   });

   if (match == std::end(*m_fileSizeOptions))
   {
      return;
   }

   const int targetIndex = m_ui.pruneSizeComboBox->findData(
      static_cast<qulonglong>(match->first));

   if (targetIndex != -1)
   {
      m_ui.pruneSizeComboBox->setCurrentIndex(targetIndex);
   }
}

void MainWindow::SetStatusBarMessage(
   const std::wstring& message,
   int timeout /* = 0*/)
{
   auto* const statusBar = this->statusBar();
   if (!statusBar)
   {
      return;
   }

   statusBar->showMessage(QString::fromStdWString(message), timeout);
}

Settings::Manager& MainWindow::GetSettingsManager()
{
   return m_settingsManager;
}

const Settings::Manager& MainWindow::GetSettingsManager() const
{
   return m_settingsManager;
}
