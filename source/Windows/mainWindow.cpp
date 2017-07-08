#include "mainWindow.h"

#include "ui_mainwindow.h"

#include "optionsManager.h"
#include "Utilities/scopeExit.hpp"
#include "Viewport/glCanvas.h"

#include <spdlog/spdlog.h>

#include <cassert>
#include <iostream>
#include <limits>
#include <sstream>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

namespace
{
   std::uint64_t GetUsedDiskSpace(std::wstring path)
   {
      std::replace(std::begin(path), std::end(path), L'/', L'\\');
      path += '\\';

      std::uint64_t totalNumberOfFreeBytes{ 0 };
      std::uint64_t totalNumberOfBytes{ 0 };
      const bool wasOperationSuccessful = GetDiskFreeSpaceExW(
         path.c_str(),
         NULL,
         (PULARGE_INTEGER)&totalNumberOfBytes,
         (PULARGE_INTEGER)&totalNumberOfFreeBytes);

      assert(wasOperationSuccessful);

      const auto& log = spdlog::get(Constants::Logging::LOG_NAME);
      log->info(fmt::format("Disk Size:  {} bytes", totalNumberOfBytes));
      log->info(fmt::format("Free Space: {} bytes", totalNumberOfFreeBytes));

      const auto occupiedSpace = totalNumberOfBytes - totalNumberOfFreeBytes;
      return occupiedSpace;
   }
}

MainWindow::MainWindow(
   Controller& controller,
   QWidget* parent /* = nullptr */)
   :
   QMainWindow(parent),
   m_controller{ controller },
   m_optionsManager(new OptionsManager),
   m_ui(new Ui::MainWindow)
{
   SetupXboxController();

   assert(m_ui);
   m_ui->setupUi(this);

   CreateMenus();

   assert(m_optionsManager);

   m_glCanvas.reset(new GLCanvas{ controller, this });
   assert(m_glCanvas);

   m_ui->canvasLayout->addWidget(m_glCanvas.get());

   SetupSidebar();
}

MainWindow::~MainWindow()
{
   delete m_ui;
}

void MainWindow::SetupSidebar()
{
   std::for_each(std::begin(m_sizePruningOptions), std::end(m_sizePruningOptions),
      [&] (const auto& pair)
   {
      m_ui->pruneSizeComboBox->addItem(pair.second, pair.first);
   });

   connect(m_ui->directoriesOnlyCheckBox, &QCheckBox::stateChanged, this, [&] (int state)
   {
      m_showDirectoriesOnly = (state == Qt::Checked);
   });

   connect(m_ui->directoryGradientCheckBox, &QCheckBox::stateChanged, this, [&] (int state)
   {
      m_useDirectoryGradient = (state == Qt::Checked);
   });

   connect(m_ui->pruneTreeButton, &QPushButton::clicked, this, [&]
   {
      const auto pruneSizeIndex = m_ui->pruneSizeComboBox->currentIndex();

      VisualizationParameters parameters;
      parameters.rootDirectory = m_directoryToVisualize;
      parameters.onlyShowDirectories = m_showDirectoriesOnly;
      parameters.useDirectoryGradient = m_useDirectoryGradient;
      parameters.forceNewScan = false;
      parameters.minimumFileSize = m_sizePruningOptions[pruneSizeIndex].first;

      m_controller.SetVisualizationParameters(parameters);

      if (!m_directoryToVisualize.empty())
      {
         m_glCanvas->ReloadVisualization();
      }
   });

   connect(m_ui->fieldOfViewSlider, &QSlider::valueChanged, this, [&] (int fieldOfView)
   {
      m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
   });

   connect(m_ui->cameraSpeedSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnCameraMovementSpeedChanged);

   connect(m_ui->mouseSensitivitySpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnMouseSensitivityChanged);

   connect(m_ui->useXBoxController,
      static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
      m_optionsManager.get(), &OptionsManager::OnUseXBoxControllerStateChanged);

   connect(m_ui->ambientCoefficientSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnAmbientCoefficientChanged);

   connect(m_ui->attenuationSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnAttenuationChanged);

   connect(m_ui->shininesSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnShininessChanged);

   connect(m_ui->attachLightToCameraCheckBox,
      static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
      m_optionsManager.get(), &OptionsManager::OnAttachLightToCameraStateChanged);

   const auto onNewSearchQuery = [&]
   {
      const bool shouldSearchFiles = m_ui->searchFilesCheckBox->isChecked();
      const bool shouldSearchDirectories = m_ui->searchDirectoriesCheckBox->isChecked();

      const auto searchQuery = m_ui->searchBox->text().toStdWString();

      const auto deselectionCallback = [&] (auto& nodes) { m_glCanvas->RestoreHighlightedNodes(nodes); };
      const auto selectionCallback = [&] (auto& nodes) { m_glCanvas->HighlightNodes(nodes); };

      m_controller.SearchTreeMap(
         searchQuery,
         deselectionCallback,
         selectionCallback,
         shouldSearchFiles,
         shouldSearchDirectories);
   };

   connect(m_ui->searchBox, &QLineEdit::returnPressed, onNewSearchQuery);

   connect(m_ui->searchButton, &QPushButton::clicked, onNewSearchQuery);

   connect(m_ui->searchDirectoriesCheckBox, &QCheckBox::stateChanged,
      m_optionsManager.get(), &OptionsManager::OnShouldSearchDirectoriesChanged);

   connect(m_ui->searchFilesCheckBox, &QCheckBox::stateChanged,
      m_optionsManager.get(), &OptionsManager::OnShouldSearchFilesChanged);

   connect(m_ui->showBreakdownButton, &QPushButton::clicked, this, [&]
   {
      if (!m_breakdownDialog)
      {
         m_breakdownDialog = std::make_unique<BreakdownDialog>(this);
      }

      m_breakdownDialog->show();
   });
}

void MainWindow::SetupXboxController()
{
   m_xboxController->StartAutoPolling(Constants::Graphics::DESIRED_TIME_BETWEEN_FRAMES);

   connect(m_xboxController.get(), SIGNAL(ControllerConnected(uint)),
      this, SLOT(XboxControllerConnected()));

   connect(m_xboxController.get(), SIGNAL(ControllerDisconnected(uint)),
      this, SLOT(XboxControllerDisconnected()));

   connect(m_xboxController.get(), SIGNAL(NewControllerState(XboxController::State)),
      this, SLOT(XboxControllerStateChanged(XboxController::State)));
}

std::wstring MainWindow::GetDirectoryToVisualize() const
{
   return m_directoryToVisualize;
}

void MainWindow::CreateMenus()
{
   CreateFileMenu();
   CreateViewMenu();
   CreateHelpMenu();
}

void MainWindow::CreateFileMenu()
{
   m_fileMenuNewScan.reset(new QAction("New Scan...", this));
   m_fileMenuNewScan->setShortcuts(QKeySequence::New);
   m_fileMenuNewScan->setStatusTip("Start a new visualization");
   connect(m_fileMenuNewScan.get(), &QAction::triggered, this, &MainWindow::OnFileMenuNewScan);

   m_fileMenuExit.reset(new QAction("Exit", this));
   m_fileMenuExit->setShortcuts(QKeySequence::Quit);
   m_fileMenuExit->setStatusTip("Exit the program");
   connect(m_fileMenuExit.get(), &QAction::triggered, this, &MainWindow::close);

   m_fileMenu.reset(menuBar()->addMenu("File"));
   m_fileMenu->addAction(m_fileMenuNewScan.get());
   m_fileMenu->addAction(m_fileMenuExit.get());
}

void MainWindow::CreateViewMenu()
{
   m_viewMenuToggleFrameTime.reset(new QAction("Show Frame Time", this));
   m_viewMenuToggleFrameTime->setCheckable(true);
   m_viewMenuToggleFrameTime->setStatusTip("Toggle Frame Time Readout");
   connect(m_viewMenuToggleFrameTime.get(), &QAction::toggled, this, &MainWindow::OnFPSReadoutToggled);

   m_viewMenu.reset(menuBar()->addMenu("View"));
   m_viewMenu->addAction(m_viewMenuToggleFrameTime.get());
}

void MainWindow::CreateHelpMenu()
{
   m_helpMenuAboutDialog.reset(new QAction("About", this));
   m_helpMenuAboutDialog->setStatusTip("About D-Viz");
   connect(m_helpMenuAboutDialog.get(), &QAction::triggered, this, &MainWindow::LaunchAboutDialog);

   m_helpMenu.reset(menuBar()->addMenu("Help"));
   m_helpMenu->addAction(m_helpMenuAboutDialog.get());
}

void MainWindow::OnFileMenuNewScan()
{
   QString selectedDirectory = QFileDialog::getExistingDirectory(
      this,
      "Select a Directory to Visualize",
      "/home",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   if (selectedDirectory.isEmpty())
   {
      return;
   }

   m_directoryToVisualize = selectedDirectory.toStdWString();

   VisualizationParameters parameters;
   parameters.rootDirectory = m_directoryToVisualize;
   parameters.onlyShowDirectories = m_showDirectoriesOnly;
   parameters.forceNewScan = true;
   parameters.minimumFileSize = m_sizePruningOptions[m_ui->pruneSizeComboBox->currentIndex()].first;

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

bool MainWindow::ShouldShowFrameTime() const
{
   return m_viewMenuToggleFrameTime->isChecked();
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

void MainWindow::LaunchAboutDialog()
{
   if (!m_aboutDialog)
   {
      m_aboutDialog = std::make_unique<AboutDialog>(this);
   }

   m_aboutDialog->show();
}

void MainWindow::XboxControllerConnected()
{
   m_xboxControllerConnected = true;
}

void MainWindow::XboxControllerDisconnected()
{
   m_xboxControllerConnected = false;
}

bool MainWindow::IsXboxControllerConnected() const
{
   return m_xboxControllerConnected;
}

void MainWindow::XboxControllerStateChanged(XboxController::State state)
{
   m_xboxControllerState = std::make_unique<XboxController::State>(std::move(state));
}

void MainWindow::ComputeProgress(
   const std::uintmax_t numberOfFilesScanned,
   const std::uintmax_t bytesProcessed)
{
   assert(m_occupiedDiskSpace > 0);

   // This clamps to percentage to just below 100%. Progress can report as being more than 100%
   // due to an issue in the std::experimental::filesystem code, which causes it to report junctions
   // as regular old directories (without ever triggering the is_symlink(...) check.

   //const auto percentComplete = std::min(99.99,
   //   100 * (static_cast<double>(bytesProcessed) / static_cast<double>(m_occupiedDiskSpace)));

   const auto percentComplete =
      100 * (static_cast<double>(bytesProcessed) / static_cast<double>(m_occupiedDiskSpace));

   std::wstringstream message;
   message.imbue(std::locale{ "" });
   message.precision(2);
   message
      << std::fixed
      << L"Files Scanned: "
      << numberOfFilesScanned
      << L"  |  "
      << percentComplete
      << L"% Complete";

   SetStatusBarMessage(message.str());
}

void MainWindow::ScanDrive(VisualizationParameters& vizParameters)
{
   m_occupiedDiskSpace = GetUsedDiskSpace(vizParameters.rootDirectory);

   const auto progressHandler =
      [this] (const std::uintmax_t numberOfFilesScanned, const std::uintmax_t bytesProcessed)
   {
      ComputeProgress(numberOfFilesScanned, bytesProcessed);
   };

   const auto completionHandler = [&, vizParameters]
      (const std::uintmax_t numberOfFilesScanned,
      const std::uintmax_t bytesProcessed,
      std::shared_ptr<Tree<VizFile>> scanningResults) mutable
   {
      const auto& log = spdlog::get(Constants::Logging::LOG_NAME);
      log->info(fmt::format("Scanned: {} files, representing {} bytes",
         numberOfFilesScanned, bytesProcessed));
      log->flush();

      ComputeProgress(numberOfFilesScanned, bytesProcessed);

      QCursor previousCursor = cursor();
      setCursor(Qt::WaitCursor);
      ON_SCOPE_EXIT{ setCursor(previousCursor); };

      QApplication::processEvents();

      AskUserToLimitFileSize(numberOfFilesScanned, vizParameters);
      m_controller.SetVisualizationParameters(vizParameters);

      m_controller.ParseResults(scanningResults);
      m_controller.UpdateBoundingBoxes();

      m_glCanvas->ReloadVisualization();

      m_controller.AllowUserInteractionWithModel(true);
      m_ui->showBreakdownButton->setEnabled(true);
   };

   const DriveScanningParameters scanningParameters
   {
      vizParameters.rootDirectory,
      progressHandler,
      completionHandler
   };

   m_controller.AllowUserInteractionWithModel(false);
   m_ui->showBreakdownButton->setEnabled(false);

   m_scanner.StartScanning(scanningParameters);
}

void MainWindow::AskUserToLimitFileSize(
   const uintmax_t numberOfFilesScanned,
   VisualizationParameters& parameters)
{
   assert(numberOfFilesScanned > 0);
   if (numberOfFilesScanned < 250'000)
   {
      return;
   }

   if (parameters.minimumFileSize < Constants::FileSize::ONE_MEBIBYTE)
   {
      QMessageBox messageBox;
      messageBox.setIcon(QMessageBox::Warning);
      messageBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
      messageBox.setText(
         "More than a quarter million files were scanned. "
         "Would you like to limit the visualized files to those 1 MiB or larger in "
         "order to reduce the load on the GPU and system memory?");

      const int election = messageBox.exec();
      switch (election)
      {
         case QMessageBox::Yes:
            parameters.minimumFileSize = Constants::FileSize::ONE_MEBIBYTE;
            SetFilePruningComboBoxValue(Constants::FileSize::ONE_MEBIBYTE);
            return;
         case QMessageBox::No:
            return;
         default:
            assert(false);
      }
   }
}

void MainWindow::SetFieldOfViewSlider(int fieldOfView)
{
   m_ui->fieldOfViewSlider->setValue(fieldOfView);
}

void MainWindow::SetCameraSpeedSpinner(double speed)
{
   m_ui->cameraSpeedSpinner->setValue(speed);
}

void MainWindow::SetFilePruningComboBoxValue(std::uintmax_t minimum)
{
   const auto match = std::find_if(std::begin(m_sizePruningOptions), std::end(m_sizePruningOptions),
      [minimum] (const auto& pair)
   {
      return pair.first >= minimum;
   });

   if (match == std::end(m_sizePruningOptions))
   {
      return;
   }

   const int targetIndex = m_ui->pruneSizeComboBox->findData(match->first);
   if (targetIndex != -1)
   {
      m_ui->pruneSizeComboBox->setCurrentIndex(targetIndex);
   }
}

XboxController::State& MainWindow::GetXboxControllerState() const
{
   return *m_xboxControllerState;
}

XboxController& MainWindow::GetXboxControllerManager()
{
   return *m_xboxController.get();
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
