#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QAction>
#include <QMenuBar>

#include "glCanvas.h"

MainWindow::MainWindow(QWidget* parent /*= 0*/, std::wstring path /*= L"C:"*/)
   : QMainWindow(parent),
     m_launchArgPath(path),
     ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   CreateMenus();

   m_glCanvas = new GLCanvas(this);
   ui->canvasLayout->addWidget(m_glCanvas);
}

MainWindow::~MainWindow()
{
   delete ui;
   delete m_glCanvas;
}

std::wstring MainWindow::GetLaunchArgPath() const
{
   return m_launchArgPath;
}

void MainWindow::CreateMenus()
{
   fileMenu = menuBar()->addMenu("&File");
   //fileMenu->addAction()
}
