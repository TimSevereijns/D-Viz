#include "mainwindow.h"

#include "glCanvas.h"
#include "ui_mainwindow.h"

#include <cassert>
#include <iostream>
#include <limits>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

MainWindow::MainWindow(QWidget* parent /*= 0*/, std::wstring path /*= L""*/)
   : QMainWindow(parent),
     m_showDirectoriesOnly(false),
     m_glCanvas(nullptr),
     m_fileMenu(nullptr),
     m_fileMenuNewScan(nullptr),
     m_fileMenuPreferences(nullptr),
     m_fileMenuExit(nullptr),
     m_directoryToVisualize(path),
     ui(new Ui::MainWindow),
     m_sizePruningComboBoxIndex(0),
     m_sizePruningOptions(
        {  // Need the "ull" (unsigned long long) prefix to avoid integral constant overflows!
           std::pair<std::uintmax_t, QString>(std::numeric_limits<std::uintmax_t>::min(), "Show All"),
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
   ui->setupUi(this);

   CreateMenus();

   m_glCanvas.reset(new GLCanvas(this));
   ui->canvasLayout->addWidget(m_glCanvas.get());

   SetupSidebar();
}

MainWindow::~MainWindow()
{
   delete ui;
}

void MainWindow::SetupSidebar()
{
   std::for_each(std::begin(m_sizePruningOptions), std::end(m_sizePruningOptions),
      [&] (const std::pair<std::uintmax_t, QString>& pair)
   {
      ui->pruneSizeComboBox->addItem(pair.second);
   });

   connect(ui->directoriesOnlyCheckbox, &QCheckBox::stateChanged, this,
      &MainWindow::OnDirectoryOnlyStateChanged);

   connect(ui->pruneTreeButton, &QPushButton::clicked, this, &MainWindow::OnPruneTreeButtonClicked);

   connect(ui->fieldOfViewSlider, &QSlider::valueChanged, this, &MainWindow::OnFieldOfViewChanged);

   connect(ui->cameraSpeedSpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), m_glCanvas.get(),
      &GLCanvas::OnCameraMovementSpeedChanged);

   connect(ui->mouseSensitivitySpinner,
      static_cast<void (QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), m_glCanvas.get(),
      &GLCanvas::OnMouseSensitivityChanged);
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

      ParsingOptions parsingOptions;
      parsingOptions.showDirectoriesOnly = false;
      parsingOptions.forceNewScan = true;
      parsingOptions.fileSizeMinimum = std::numeric_limits<std::uintmax_t>::min();

      m_glCanvas->ParseVisualization(m_directoryToVisualize, parsingOptions);
   }
}

void MainWindow::OnDirectoryOnlyStateChanged(int state)
{
   m_showDirectoriesOnly = (state == Qt::Checked);
}

void MainWindow::OnPruneTreeButtonClicked()
{
   ParsingOptions parsingOptions;
   parsingOptions.showDirectoriesOnly = m_showDirectoriesOnly;
   parsingOptions.forceNewScan = false;
   parsingOptions.fileSizeMinimum = m_sizePruningOptions[ui->pruneSizeComboBox->currentIndex()].first;

   m_glCanvas->ParseVisualization(m_directoryToVisualize, parsingOptions);

   std::cout << "Button pressed" << std::endl;
}

void MainWindow::OnFieldOfViewChanged(const int fieldOfView)
{
   m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
}
