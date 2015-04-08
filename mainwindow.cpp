#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenuBar>

#include "glCanvas.h"

MainWindow::MainWindow(QWidget* parent /*= 0*/, std::wstring path /*= L"C:"*/)
   : QMainWindow(parent),
     m_glCanvas(nullptr),
     m_fileMenu(nullptr),
     m_fileMenuNewScan(nullptr),
     m_fileMenuPreferences(nullptr),
     m_fileMenuExit(nullptr),
     m_launchArgPath(path),
     ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   CreateMenus();

   m_glCanvas.reset(new GLCanvas(this));
   ui->canvasLayout->addWidget(m_glCanvas.get());
}

MainWindow::~MainWindow()
{
   delete ui;
}

std::wstring MainWindow::GetLaunchArgPath() const
{
   return m_launchArgPath;
}

void MainWindow::CreateMenus()
{
   m_fileMenuNewScan.reset(new QAction("New Scan...", this));
   m_fileMenuNewScan->setShortcuts(QKeySequence::New);
   m_fileMenuNewScan->setStatusTip("Start a new visualization");
   connect(m_fileMenuNewScan.get(), SIGNAL(triggered()), this, SLOT([](){}));

   m_fileMenuPreferences.reset(new QAction("Preferences...", this));
   m_fileMenuPreferences->setShortcuts(QKeySequence::Preferences);
   m_fileMenuPreferences->setStatusTip("Tweak program settings");
   connect(m_fileMenuPreferences.get(), SIGNAL(triggered()), this, SLOT([](){}));

   m_fileMenuExit.reset(new QAction("Exit", this));
   m_fileMenuExit->setShortcuts(QKeySequence::Quit);
   m_fileMenuExit->setStatusTip("Exit the program");
   connect(m_fileMenuExit.get(), SIGNAL(triggered()), this, SLOT(close()));

   m_fileMenu.reset(menuBar()->addMenu("File"));
   m_fileMenu->addAction(m_fileMenuNewScan.get());
   m_fileMenu->addAction(m_fileMenuPreferences.get());
   m_fileMenu->addAction(m_fileMenuExit.get());
}
