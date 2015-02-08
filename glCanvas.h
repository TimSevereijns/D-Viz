#ifndef GLCANVAS_H
#define GLCANVAS_H

#include <QGLWidget>
#include <QGLShaderProgram>

class GLCanvas : public QGLWidget
{
   Q_OBJECT

   public:
      explicit GLCanvas(QWidget* parent = nullptr);
      ~GLCanvas();

      QSize sizeHint() const;

   protected:
      void initializeGL() override;
      void resizeGL(int width, int height) override;
      void paintGL() override;

   private:
      QMatrix4x4 m_projectionMatrix;
      QGLShaderProgram m_shader;
      QVector<QVector3D> m_vertices;

   signals:

   public slots:
};

#endif // GLCANVAS_H
