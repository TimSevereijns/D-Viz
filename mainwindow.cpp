#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "glCanvas.h"

MainWindow::MainWindow(QWidget* parent /*= 0*/, std::wstring path /*= L"C:"*/)
   : QMainWindow(parent),
     m_launchArgPath(path),
     ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   m_glCanvas = new GLCanvas(this);
   ui->canvasLayout->addWidget(m_glCanvas);
}

MainWindow::~MainWindow()
{
   delete ui;
}

void MainWindow::resizeEvent(QResizeEvent* /*event*/)
{
   m_glCanvas->resize(size().width(), size().height());
}

std::wstring MainWindow::GetLaunchArgPath() const
{
   return m_launchArgPath;
}
