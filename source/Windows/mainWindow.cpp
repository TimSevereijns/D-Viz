#include "mainWindow.h"

#include "constants.h"
#include "DataStructs/scanningProgress.hpp"
#include "optionsManager.h"
#include "Utilities/operatingSystemSpecific.hpp"
#include "Utilities/scopeExit.hpp"
#include "Viewport/glCanvas.h"

#include <boost/algorithm/string.hpp>
#include <spdlog/spdlog.h>

#include <cassert>
#include <iostream>
#include <limits>
#include <memory>
#include <sstream>

#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

Constants::FileSize::Prefix ActivePrefix = Constants::FileSize::Prefix::BINARY;

namespace
{
   void LogScanCompletion(const ScanningProgress& progress)
   {
      const auto filesScanned = progress.filesScanned.load();
      const auto directoriesScanned = progress.directoriesScanned.load();
      const auto bytesProcessed = progress.bytesProcessed.load();

      const auto& log = spdlog::get(Constants::Logging::LOG_NAME);

      log->info(
         fmt::format("Scanned: {} directories and {} files, representing {} bytes",
         directoriesScanned,
         filesScanned,
         bytesProcessed));

      log->flush();
   }
}

MainWindow::MainWindow(
   Controller& controller,
   QWidget* parent /* = nullptr */)
   :
   QMainWindow{ parent },
   m_controller{ controller },
   m_optionsManager{ std::make_shared<OptionsManager>() }
{
   m_ui.setupUi(this);

   m_glCanvas = std::make_unique<GLCanvas>(controller, this);
   m_ui.canvasLayout->addWidget(m_glCanvas.get());

   SetupMenus();
   SetupGamepad();
   SetupSidebar();
}

void MainWindow::SetupSidebar()
{
   SetupFileSizePruningDropdown();

   connect(m_ui.directoriesOnlyCheckBox, &QCheckBox::stateChanged, this, [&] (int state)
   {
      m_showDirectoriesOnly = (state == Qt::Checked);
   });

   connect(m_ui.directoryGradientCheckBox, &QCheckBox::stateChanged, this, [&] (int state)
   {
      m_useDirectoryGradient = (state == Qt::Checked);
   });

   connect(m_ui.pruneTreeButton, &QPushButton::clicked, this, [&]
   { // @todo Consider moving this logic onto the Controller...
      const auto pruneSizeIndex = m_ui.pruneSizeComboBox->currentIndex();

      VisualizationParameters parameters;
      parameters.rootDirectory = m_rootPath;
      parameters.onlyShowDirectories = m_showDirectoriesOnly;
      parameters.useDirectoryGradient = m_useDirectoryGradient;
      parameters.forceNewScan = false;
      parameters.minimumFileSize = m_fileSizeOptions[pruneSizeIndex].first;

      m_controller.SetVisualizationParameters(parameters);

      if (!m_rootPath.empty())
      {
         m_glCanvas->ReloadVisualization();
      }
   });

   connect(m_ui.fieldOfViewSlider, &QSlider::valueChanged, this, [&] (int fieldOfView)
   {
      m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
   });

   connect(m_ui.cameraSpeedSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnCameraMovementSpeedChanged);

   connect(m_ui.mouseSensitivitySpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnMouseSensitivityChanged);

   connect(m_ui.ambientCoefficientSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnAmbientCoefficientChanged);

   connect(m_ui.attenuationSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnAttenuationChanged);

   connect(m_ui.shininesSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnShininessChanged);

   connect(m_ui.attachLightToCameraCheckBox,
      static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
      m_optionsManager.get(), &OptionsManager::OnAttachLightToCameraStateChanged);

   const auto onNewSearchQuery = [&]
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

      const bool shouldSearchFiles = m_ui.searchFilesCheckBox->isChecked();
      const bool shouldSearchDirectories = m_ui.searchDirectoriesCheckBox->isChecked();

      m_controller.SearchTreeMap(
         searchQuery,
         deselectionCallback,
         selectionCallback,
         shouldSearchFiles,
         shouldSearchDirectories);
   };

   connect(m_ui.searchBox, &QLineEdit::returnPressed, onNewSearchQuery);

   connect(m_ui.searchButton, &QPushButton::clicked, onNewSearchQuery);

   connect(m_ui.searchDirectoriesCheckBox, &QCheckBox::stateChanged,
      m_optionsManager.get(), &OptionsManager::OnShouldSearchDirectoriesChanged);

   connect(m_ui.searchFilesCheckBox, &QCheckBox::stateChanged,
      m_optionsManager.get(), &OptionsManager::OnShouldSearchFilesChanged);

   connect(m_ui.showBreakdownButton, &QPushButton::clicked, this, [&]
   {
      if (!m_breakdownDialog)
      {
         m_breakdownDialog = std::make_unique<BreakdownDialog>(this);
      }

      m_breakdownDialog->show();
   });
}

void MainWindow::SetupFileSizePruningDropdown()
{
   m_ui.pruneSizeComboBox->clear();

   std::for_each(std::begin(m_fileSizeOptions), std::end(m_fileSizeOptions),
      [&] (const auto& pair)
   {
      m_ui.pruneSizeComboBox->addItem(pair.second, static_cast<qulonglong>(pair.first));
   });

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
   return m_rootPath;
}

void MainWindow::SetupMenus()
{
   SetupFileMenu();
   SetupOptionsMenu();
   SetupHelpMenu();
}

void MainWindow::SetupFileMenu()
{
   m_fileMenuWrapper.newScan.setParent(this);
   m_fileMenuWrapper.newScan.setText("New Scan...");
   m_fileMenuWrapper.newScan.setStatusTip("Start a new visualization.");
   m_fileMenuWrapper.newScan.setShortcuts(QKeySequence::New);

   connect(&m_fileMenuWrapper.newScan, &QAction::triggered, this, &MainWindow::OnFileMenuNewScan);

   m_fileMenuWrapper.exit.setParent(this);
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
   m_optionsMenuWrapper.toggleFrameTime.setParent(this);
   m_optionsMenuWrapper.toggleFrameTime.setText("Show Frame Time");
   m_optionsMenuWrapper.toggleFrameTime.setStatusTip("Toggle frame-time readout in titlebar.");
   m_optionsMenuWrapper.toggleFrameTime.setCheckable(true);

   connect(&m_optionsMenuWrapper.toggleFrameTime, &QAction::toggled, this,
      &MainWindow::OnFPSReadoutToggled);

   m_optionsMenu.setTitle("Options");
   m_optionsMenu.addAction(&m_optionsMenuWrapper.toggleFrameTime);

   SetupFileSizeSubMenu();

   menuBar()->addMenu(&m_optionsMenu);
}

void MainWindow::SetupFileSizeSubMenu()
{
   auto& subMenuWrapper = m_optionsMenuWrapper.fileSizeMenuWrapper;

   subMenuWrapper.binaryPrefix.setParent(&m_optionsMenu);
   subMenuWrapper.binaryPrefix.setText("Binary Prefix");
   subMenuWrapper.binaryPrefix.setStatusTip(
      "Use base-two units, such as kibibytes and mebibytes. This is the default on Windows.");
   subMenuWrapper.binaryPrefix.setCheckable(true);
   subMenuWrapper.binaryPrefix.setChecked(true);

   connect(&subMenuWrapper.binaryPrefix, &QAction::toggled, this,
      &MainWindow::SwitchToBinaryPrefix);

   subMenuWrapper.decimalPrefix.setParent(&m_optionsMenu);
   subMenuWrapper.decimalPrefix.setText("Decimal Prefix");
   subMenuWrapper.decimalPrefix.setStatusTip(
      "Use base-ten units, such as kilobytes and megabytes.");
   subMenuWrapper.decimalPrefix.setCheckable(true);
   subMenuWrapper.decimalPrefix.setChecked(false);

   connect(&subMenuWrapper.decimalPrefix, &QAction::toggled, this,
      &MainWindow::SwitchToDecimalPrefix);

   m_optionsMenuWrapper.fileSizeMenu.setTitle("File Size Units");
   m_optionsMenuWrapper.fileSizeMenu.addAction(&subMenuWrapper.binaryPrefix);
   m_optionsMenuWrapper.fileSizeMenu.addAction(&subMenuWrapper.decimalPrefix);

   m_optionsMenu.addMenu(&m_optionsMenuWrapper.fileSizeMenu);
}

void MainWindow::SetupHelpMenu()
{
   m_helpMenuWrapper.aboutDialog.setParent(this);
   m_helpMenuWrapper.aboutDialog.setText("About");
   m_helpMenuWrapper.aboutDialog.setStatusTip("About D-Viz");

   connect(&m_helpMenuWrapper.aboutDialog, &QAction::triggered, this,
      &MainWindow::LaunchAboutDialog);

   m_helpMenu.setTitle("Help");
   m_helpMenu.addAction(&m_helpMenuWrapper.aboutDialog);

   menuBar()->addMenu(&m_helpMenu);
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
      return;
   }

   m_rootPath = selectedDirectory.toStdWString();

   const auto& log = spdlog::get(Constants::Logging::LOG_NAME);
   log->info(fmt::format("Started a new scan at: \"{}\"", m_rootPath.string()));

   const auto fileSizeIndex = m_ui.pruneSizeComboBox->currentIndex();

   VisualizationParameters parameters;
   parameters.rootDirectory = m_rootPath;
   parameters.onlyShowDirectories = m_showDirectoriesOnly;
   parameters.forceNewScan = true;
   parameters.minimumFileSize = m_fileSizeOptions[fileSizeIndex].first;

   m_controller.SetVisualizationParameters(parameters);
   m_controller.GenerateNewVisualization();
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
   // @todo emit a signal that the breakdown dialog can hook up to.

   m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(true);
   m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(true);

   ON_SCOPE_EXIT
   {
      m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(false);
      m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(false);
   };

   m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.setChecked(true);
   m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.setChecked(false);

   ActivePrefix = Constants::FileSize::Prefix::BINARY;

   m_fileSizeOptions = m_binaryFileSizeOptions;
   SetupFileSizePruningDropdown();
}

void MainWindow::SwitchToDecimalPrefix(bool /*useDecimal*/)
{
   // @todo emit a signal that the breakdown dialog can hook up to.

   m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(true);
   m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(true);

   ON_SCOPE_EXIT
   {
      m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.blockSignals(false);
      m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.blockSignals(false);
   };

   m_optionsMenuWrapper.fileSizeMenuWrapper.binaryPrefix.setChecked(false);
   m_optionsMenuWrapper.fileSizeMenuWrapper.decimalPrefix.setChecked(true);

   ActivePrefix = Constants::FileSize::Prefix::DECIMAL;

   m_fileSizeOptions = m_decimalFileSizeOptions;
   SetupFileSizePruningDropdown();
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
   return *m_glCanvas;
}

Gamepad& MainWindow::GetGamepad()
{
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
      // directories instead of some type of link. This means that any junctions will be scanned and
      // counted against the total.

      const auto percentComplete =
         100 * (static_cast<double>(bytesProcessed) / static_cast<double>(m_occupiedDiskSpace));

      std::wstringstream message;
      message.imbue(std::locale{ "" });
      message.precision(2);
      message
         << std::fixed
         << L"Files Scanned: "
         << filesScanned
         << L"  |  "
         << percentComplete
         << L"% Complete";

      SetStatusBarMessage(message.str());
   }
   else
   {
      const auto sizeAndUnits = Controller::ConvertFileSizeToAppropriateUnits(bytesProcessed);

      std::wstringstream message;
      message.imbue(std::locale{ "" });
      message.precision(2);
      message
         << std::fixed
         << L"Files Scanned: " << filesScanned
         << L"  |  "
         << sizeAndUnits.first
         << L" "
         << sizeAndUnits.second
         << " and counting...";

      SetStatusBarMessage(message.str());
   }
}

void MainWindow::ScanDrive(VisualizationParameters& vizParameters)
{
   m_occupiedDiskSpace = OperatingSystemSpecific::GetUsedDiskSpace(vizParameters.rootDirectory);

   const auto progressHandler = [this] (const ScanningProgress& progress)
   {
      ComputeProgress(progress);
   };

   const auto completionHandler = [&, vizParameters] (
      const ScanningProgress& progress,
      std::shared_ptr<Tree<VizFile>> scanningResults) mutable
   {
      ComputeProgress(progress);

      LogScanCompletion(progress);

      m_controller.SaveScanResults(progress);

      QCursor previousCursor = cursor();
      setCursor(Qt::WaitCursor);
      ON_SCOPE_EXIT{ setCursor(previousCursor); };

      QApplication::processEvents();

      const auto filesScanned = progress.filesScanned.load();
      AskUserToLimitFileSize(filesScanned, vizParameters);

      m_controller.SetVisualizationParameters(vizParameters);

      m_controller.ParseResults(scanningResults);
      m_controller.UpdateBoundingBoxes();

      m_glCanvas->ReloadVisualization();

      m_controller.AllowUserInteractionWithModel(true);
      m_ui.showBreakdownButton->setEnabled(true);
   };

   const DriveScanningParameters scanningParameters
   {
      vizParameters.rootDirectory,
      progressHandler,
      completionHandler
   };

   m_controller.AllowUserInteractionWithModel(false);
   m_ui.showBreakdownButton->setEnabled(false);

   m_scanner.StartScanning(scanningParameters);
}

void MainWindow::AskUserToLimitFileSize(
   std::uintmax_t numberOfFilesScanned,
   VisualizationParameters& parameters)
{
   assert(numberOfFilesScanned > 0);

   if (numberOfFilesScanned < 250'000
      || parameters.minimumFileSize >= Constants::FileSize::Binary::ONE_MEBIBYTE)
   {
      return;
   }

   QMessageBox messageBox;
   messageBox.setIcon(QMessageBox::Warning);
   messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
   messageBox.setText(
      "More than a quarter million files were scanned. "
      "Would you like to limit the visualized files to those 1 MiB or larger in "
      "order to reduce the load on the GPU and system memory?");

   const auto election = messageBox.exec();
   switch (election)
   {
      case QMessageBox::Yes:
         parameters.minimumFileSize = Constants::FileSize::Binary::ONE_MEBIBYTE;
         SetFilePruningComboBoxValue(Constants::FileSize::Binary::ONE_MEBIBYTE);
         return;
      case QMessageBox::No:
         return;
      default:
         assert(false);
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
      std::begin(m_fileSizeOptions),
      std::end(m_fileSizeOptions),
      [minimum] (const auto& pair) noexcept
   {
      return pair.first >= minimum;
   });

   if (match == std::end(m_fileSizeOptions))
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
   auto* statusBar = this->statusBar();
   if (!statusBar)
   {
      return;
   }

   statusBar->showMessage(QString::fromStdWString(message), timeout);
}

std::shared_ptr<OptionsManager> MainWindow::GetOptionsManager()
{
   return m_optionsManager;
}
