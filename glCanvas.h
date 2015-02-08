#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <QGLWidget>

class GLCanvas : public QGLWidget
{
   Q_OBJECT

   public:
      explicit GLCanvas(QWidget *parent = 0);
      ~GLCanvas();

   protected:


   signals:

   public slots:
};

#endif // GLCANVAS_H
