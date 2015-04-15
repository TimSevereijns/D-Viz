#include "mainwindow.h"

#include "glCanvas.h"
#include "ui_mainwindow.h"

#include <cassert>
#include <iostream>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

namespace
{
   /**
    * @brief SideBarSetupHelper performs all tasks necessary to setup the sidebar.
    *
    * @param[in/out] ui                The UI on which all the controls exist.
    */
   void SetupPruneSizeComboBox(Ui::MainWindow& ui)
   {
      // Setup the pruning options:
      ui.pruneSizeComboBox->addItem("Show All");
      ui.pruneSizeComboBox->addItem("< 1 MB");
      ui.pruneSizeComboBox->addItem("< 10 MB");
      ui.pruneSizeComboBox->addItem("< 50 MB");
      ui.pruneSizeComboBox->addItem("< 100 MB");
      ui.pruneSizeComboBox->addItem("< 250 MB");
      ui.pruneSizeComboBox->addItem("< 500 MB");
      ui.pruneSizeComboBox->addItem("< 1 GB");
      ui.pruneSizeComboBox->addItem("< 5 GB");
      ui.pruneSizeComboBox->addItem("< 10 GB");
   }
}

MainWindow::MainWindow(QWidget* parent /*= 0*/, std::wstring path /*= L""*/)
   : QMainWindow(parent),
     m_showDirectoriesOnly(false),
     m_glCanvas(nullptr),
     m_fileMenu(nullptr),
     m_fileMenuNewScan(nullptr),
     m_fileMenuPreferences(nullptr),
     m_fileMenuExit(nullptr),
     m_directoryToVisualize(path),
     ui(new Ui::MainWindow)
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
   SetupPruneSizeComboBox(*ui);

   connect(ui->directoriesOnlyCheckbox, &QCheckBox::stateChanged, this,
      &MainWindow::OnDirectoryOnlyStateChanged);
   connect(ui->pruneTreeButton, &QPushButton::clicked, this, &MainWindow::OnPruneTreeButtonClicked);
   connect(ui->fieldOfViewSlider, &QSlider::valueChanged, this, &MainWindow::OnFieldOfViewChanged);
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

   m_glCanvas->ParseVisualization(m_directoryToVisualize, parsingOptions);

   std::cout << "Button pressed" << std::endl;
}

void MainWindow::OnFieldOfViewChanged(const int fieldOfView)
{
   m_glCanvas->SetFieldOfView(static_cast<float>(fieldOfView));
}
