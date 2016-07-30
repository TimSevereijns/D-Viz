#include "mainWindow.h"

#include "optionsManager.h"
#include "Viewport/glCanvas.h"

#include <cassert>
#include <iostream>
#include <limits>

#include "ui_mainwindow.h"

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QMessageBox>

MainWindow::MainWindow(QWidget* parent /* = nullptr */) :
   QMainWindow(parent),
   m_optionsManager(new OptionsManager),
   m_ui(new Ui::MainWindow)
{
   SetupXboxController();

   m_ui->setupUi(this);

   CreateMenus();

   assert(m_optionsManager);

   m_glCanvas.reset(new GLCanvas{ m_model, this });
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

   connect(m_ui->directoriesOnlyCheckBox, &QCheckBox::stateChanged, this,
      [&] (int state)
   {
      m_showDirectoriesOnly = (state == Qt::Checked);
   });

   connect(m_ui->directoryGradientCheckBox, &QCheckBox::stateChanged, this,
      [&] (int state)
   {
      m_useDirectoryGradient = (state == Qt::Checked);
   });

   connect(m_ui->pruneTreeButton, &QPushButton::clicked, this,
      [&] ()
   {
      const auto pruneSizeIndex = m_ui->pruneSizeComboBox->currentIndex();

      VisualizationParameters parameters;
      parameters.rootDirectory = m_directoryToVisualize;
      parameters.onlyShowDirectories = m_showDirectoriesOnly;
      parameters.useDirectoryGradient = m_useDirectoryGradient;
      parameters.forceNewScan = false;
      parameters.minimumFileSize = m_sizePruningOptions[pruneSizeIndex].first;

      if (!m_directoryToVisualize.empty())
      {
         m_glCanvas->ReloadVisualization(parameters);
      }
   });

   connect(m_ui->fieldOfViewSlider, &QSlider::valueChanged, this,
      [&] (int fieldOfView)
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

   connect(m_ui->regexSearchBox, &QLineEdit::returnPressed,
      m_glCanvas.get(), &GLCanvas::PerformNodeSearch);

   connect(m_ui->regexSearchButton, &QPushButton::clicked,
      m_glCanvas.get(), &GLCanvas::PerformNodeSearch);

   connect(m_ui->regexSearchBox, &QLineEdit::textChanged, this,
      [&] (const auto& newText)
   {
      m_searchQuery = newText.toStdWString();
   });

   connect(m_ui->searchDirectoriesCheckBox, &QCheckBox::stateChanged,
      m_optionsManager.get(), &OptionsManager::OnShouldSearchDirectoriesChanged);

   connect(m_ui->searchFilesCheckBox, &QCheckBox::stateChanged,
      m_optionsManager.get(), &OptionsManager::OnShouldSearchFilesChanged);

   connect(m_ui->showBreakdownButton, &QPushButton::clicked, this,
      [&] ()
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
   m_viewMenuToggleFPS.reset(new QAction("Show FPS", this));
   m_viewMenuToggleFPS->setCheckable(true);
   m_viewMenuToggleFPS->setStatusTip("Toggle FPS Readout");
   connect(m_viewMenuToggleFPS.get(), &QAction::toggled, this, &MainWindow::OnFPSReadoutToggled);

   m_viewMenu.reset(menuBar()->addMenu("View"));
   m_viewMenu->addAction(m_viewMenuToggleFPS.get());
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

   const auto comboBoxIndex = m_ui->pruneSizeComboBox->currentIndex();

   VisualizationParameters parameters;
   parameters.rootDirectory = m_directoryToVisualize;
   parameters.onlyShowDirectories = m_showDirectoriesOnly;
   parameters.forceNewScan = true;
   parameters.minimumFileSize = m_sizePruningOptions[comboBoxIndex].first;

   //m_glCanvas->CreateNewVisualization(parameters);
}

void MainWindow::OnFPSReadoutToggled(bool isEnabled)
{
   if (!isEnabled)
   {
      setWindowTitle("D-Viz [*]");
   }
}

bool MainWindow::ShouldShowFPS() const
{
   return m_viewMenuToggleFPS->isChecked();
}

std::wstring MainWindow::GetSearchQuery() const
{
   return m_searchQuery;
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
