#include "mainwindow.h"

#include "optionsManager.h"
#include "Viewport/glCanvas.h"

#include <cassert>
#include <iostream>
#include <limits>

#include "ui_mainwindow.h"

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent /* = 0 */) :
   QMainWindow(parent),
   m_optionsManager(new OptionsManager),
   m_ui(new Ui::MainWindow)
{
   SetupXboxController();

   m_ui->setupUi(this);

   CreateMenus();

   assert(m_optionsManager);

   m_glCanvas.reset(new GLCanvas{ this });
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

   connect(m_ui->directoriesOnlyCheckbox, &QCheckBox::stateChanged,
      this, &MainWindow::OnDirectoryOnlyStateChanged);

   connect(m_ui->directoryGradientCheckBox, &QCheckBox::stateChanged,
      this, &MainWindow::OnDirectoryGradientStateChanged);

   connect(m_ui->pruneTreeButton, &QPushButton::clicked,
      this, &MainWindow::OnPruneTreeButtonClicked);

   connect(m_ui->fieldOfViewSlider, &QSlider::valueChanged,
      this, &MainWindow::OnFieldOfViewChanged);

   connect(m_ui->cameraSpeedSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnCameraMovementSpeedChanged);

   connect(m_ui->mouseSensitivitySpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnMouseSensitivityChanged);

   connect(m_ui->ambientCoefficientSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnAmbientCoefficientChanged);

   connect(m_ui->attenuationSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnAttenuationChanged);

   connect(m_ui->shininesSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
      m_optionsManager.get(), &OptionsManager::OnShininessChanged);

   connect(m_ui->lightRedSlider, &QSlider::valueChanged,
      m_optionsManager.get(), &OptionsManager::OnRedLightComponentChanged);

   connect(m_ui->lightGreenSlider, &QSlider::valueChanged,
      m_optionsManager.get(), &OptionsManager::OnGreenLightComponentChanged);

   connect(m_ui->lightBlueSlider, &QSlider::valueChanged,
      m_optionsManager.get(), &OptionsManager::OnBlueLightComponentChanged);

   connect(m_ui->attachLightToCameraCheckBox,
      static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
      m_optionsManager.get(), &OptionsManager::OnAttachLightToCameraStateChanged);

   connect(m_ui->useXBoxController,
      static_cast<void (QCheckBox::*)(int)>(&QCheckBox::stateChanged),
      m_optionsManager.get(), &OptionsManager::OnUseXBoxControllerStateChanged);
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

   m_glCanvas->CreateNewVisualization(parameters);
}

void MainWindow::OnDirectoryOnlyStateChanged(int state)
{
   m_showDirectoriesOnly = (state == Qt::Checked);
}

void MainWindow::OnDirectoryGradientStateChanged(int state)
{
   m_useDirectoryGradient = (state == Qt::Checked);
}

void MainWindow::OnPruneTreeButtonClicked()
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

void MainWindow::OnFieldOfViewChanged(const int fieldOfView)
{
   m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
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

void MainWindow::SetStatusBarMessage(const std::wstring& message)
{
   auto* statusBar = this->statusBar();
   if (!statusBar)
   {
      return;
   }

   statusBar->showMessage(QString::fromStdWString(message));
}

std::shared_ptr<OptionsManager> MainWindow::GetOptionsManager()
{
   return m_optionsManager;
}
