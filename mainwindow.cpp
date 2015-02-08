#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "glCanvas.h"

MainWindow::MainWindow(QWidget* parent)
   : QMainWindow(parent),
   ui(new Ui::MainWindow)
{
   ui->setupUi(this);

   glCanvas = new GLCanvas(this);
   ui->canvasLayout->addWidget(glCanvas);
}

MainWindow::~MainWindow()
{
   delete ui;
}
