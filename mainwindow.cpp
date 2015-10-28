#include "mainwindow.h"

#include "optionsManager.h"
#include "ui_mainwindow.h"
#include "Viewport/glCanvas.h"

#include <cassert>
#include <iostream>
#include <limits>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent /*= 0*/)
   : QMainWindow(parent),
     m_showDirectoriesOnly(false),
     m_xboxControllerConnected(false),
     m_xboxController(new XboxController(0)),
     m_xboxControllerState(nullptr),
     m_glCanvas(nullptr),
     m_optionsManager(new OptionsManager()),
     m_fileMenu(nullptr),
     m_fileMenuNewScan(nullptr),
     m_fileMenuPreferences(nullptr),
     m_fileMenuExit(nullptr),
     m_directoryToVisualize(L""),
     m_ui(new Ui::MainWindow),
     m_sizePruningComboBoxIndex(0),
     m_sizePruningOptions(
        {  // Need the "ull" (unsigned long long) prefix to avoid integral constant overflows!
           std::pair<std::uintmax_t, QString>(0, "Show All"),
           std::pair<std::uintmax_t, QString>(1024ull,            "< 1 Kib"),
           std::pair<std::uintmax_t, QString>(1048576ull,         "< 1 MiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 10,    "< 10 MiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 100,   "< 100 MiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 250,   "< 250 MiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 500,   "< 500 MiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 1000,  "< 1 GiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 5000,  "< 5 GiB"),
           std::pair<std::uintmax_t, QString>(1048576ull * 10000, "< 10 GiB")
        })
{
   SetupXboxController();

   m_ui->setupUi(this);

   CreateMenus();

   assert(m_optionsManager);

   m_glCanvas.reset(new GLCanvas(this));
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
      [&] (const std::pair<std::uintmax_t, QString>& pair)
   {
      m_ui->pruneSizeComboBox->addItem(pair.second);
   });

   connect(m_ui->directoriesOnlyCheckbox, &QCheckBox::stateChanged,
      this, &MainWindow::OnDirectoryOnlyStateChanged);

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
   m_xboxController->StartAutoPolling(20);

   m_xboxController->SetDownHandler(XINPUT_GAMEPAD_Y,
      [] () { std::cout << "Button Y pressed..." << std::endl; });

   m_xboxController->SetUpHandler(XINPUT_GAMEPAD_Y,
      [] () { std::cout << "Button Y released..." << std::endl; });

   connect(&*m_xboxController, SIGNAL(ControllerConnected(uint)),
      this, SLOT(XboxControllerConnected()));

   connect(&*m_xboxController, SIGNAL(ControllerDisconnected(uint)),
      this, SLOT(XboxControllerDisconnected()));

   connect(&*m_xboxController, SIGNAL(NewControllerState(XboxController::State)),
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

   m_fileMenuPreferences.reset(new QAction("Preferences...", this));
   m_fileMenuPreferences->setShortcuts(QKeySequence::Preferences);
   m_fileMenuPreferences->setStatusTip("Tweak program settings");
   connect(m_fileMenuPreferences.get(), &QAction::triggered, this, [](){});

   m_fileMenuExit.reset(new QAction("Exit", this));
   m_fileMenuExit->setShortcuts(QKeySequence::Quit);
   m_fileMenuExit->setStatusTip("Exit the program");
   connect(m_fileMenuExit.get(), &QAction::triggered, this, &MainWindow::close);

   m_fileMenu.reset(menuBar()->addMenu("File"));
   m_fileMenu->addAction(m_fileMenuNewScan.get());
   m_fileMenu->addAction(m_fileMenuPreferences.get());
   m_fileMenu->addAction(m_fileMenuExit.get());
}

void MainWindow::CreateWindowMenu()
{
   // TODO
}

void MainWindow::OnFileMenuNewScan()
{
   QString selectedDirectory = QFileDialog::getExistingDirectory(this,
      "Select a Directory to Visualize", "/home",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   if (!selectedDirectory.isEmpty())
   {
      m_directoryToVisualize = selectedDirectory.toStdWString();

      const auto pruneSizeIndex = m_ui->pruneSizeComboBox->currentIndex();

      VisualizationParameters parameters;
      parameters.rootDirectory = m_directoryToVisualize;
      parameters.onlyShowDirectories = m_showDirectoriesOnly;
      parameters.forceNewScan = true;
      parameters.minimumFileSize = m_sizePruningOptions[pruneSizeIndex].first;

      m_glCanvas->CreateNewVisualization(parameters);
   }
}

void MainWindow::OnDirectoryOnlyStateChanged(int state)
{
   m_showDirectoriesOnly = (state == Qt::Checked);
}

void MainWindow::OnPruneTreeButtonClicked()
{
   const auto pruneSizeIndex = m_ui->pruneSizeComboBox->currentIndex();

   VisualizationParameters parameters;
   parameters.rootDirectory = m_directoryToVisualize;
   parameters.onlyShowDirectories = m_showDirectoriesOnly;
   parameters.forceNewScan = false;
   parameters.minimumFileSize = m_sizePruningOptions[pruneSizeIndex].first;

   // Allow the "Apply" button to be clicked even if nothing is loaded:
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
   m_xboxControllerState.reset(new XboxController::State(state));
}

void MainWindow::OnFieldOfViewChanged(const int fieldOfView)
{
   m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
}

void MainWindow::UpdateFieldOfViewSlider(const int fieldOfView)
{
   m_ui->fieldOfViewSlider->setValue(fieldOfView);
}

XboxController::State& MainWindow::GetXboxControllerState() const
{
   return *m_xboxControllerState;
}

XboxController& MainWindow::GetXboxControllerManager()
{
   return *m_xboxController.get();
}

std::shared_ptr<OptionsManager> MainWindow::GetOptionsManager()
{
   return m_optionsManager;
}
