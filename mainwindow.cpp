#include "mainwindow.h"

#include "glCanvas.h"
#include "ui_mainwindow.h"

#include <cassert>
#include <iostream>

#include <QAction>
#include <QFileDialog>
#include <QMenuBar>

/**
 * @brief SideBarSetupHelper performs all tasks necessary to setup the sidebar.
 *
 * @param[in/out] ui                The UI on which all the controls exist.
 */
void SideBarSetupHelper(Ui::MainWindow& ui)
{
   ui.comboBox->addItem("Show All");
   ui.comboBox->addItem("< 1 MB");
   ui.comboBox->addItem("< 10 MB");
   ui.comboBox->addItem("< 50 MB");
   ui.comboBox->addItem("< 100 MB");
   ui.comboBox->addItem("< 250 MB");
   ui.comboBox->addItem("< 500 MB");
   ui.comboBox->addItem("< 1 GB");
   ui.comboBox->addItem("< 5 GB");
   ui.comboBox->addItem("< 10 GB");
}

MainWindow::MainWindow(QWidget* parent /*= 0*/, std::wstring path /*= L""*/)
   : QMainWindow(parent),
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

   SideBarSetupHelper(*ui);
}

MainWindow::~MainWindow()
{
   delete ui;
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
   connect(m_fileMenuNewScan.get(), &QAction::triggered, this, &MainWindow::HandleFileMenuNewScan);

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

void MainWindow::HandleFileMenuNewScan()
{
   QString selectedDirectory = QFileDialog::getExistingDirectory(this,
      "Select a Directory to Visualize", "/home",
      QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

   if (!selectedDirectory.isEmpty())
   {
      m_directoryToVisualize = selectedDirectory.toStdWString();
      m_glCanvas->ParseVisualization(m_directoryToVisualize);
   }
}
