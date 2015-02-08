#include "glCanvas.h"

#include <QGLWidget>

GLCanvas::GLCanvas(QWidget *parent) : QGLWidget(parent)
{
}

GLCanvas::~GLCanvas()
{
}

QSize GLCanvas::sizeHint() const
{
   return QSize(780, 580);
}

void GLCanvas::initializeGL()
{
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_CULL_FACE);

   qglClearColor(QColor(Qt::black));

   m_shader.addShaderFromSourceFile(QGLShader::Vertex, ":/Shaders/vertexshader.vert");
   m_shader.addShaderFromSourceFile(QGLShader::Fragment, ":/Shaders/fragmentShader.frag");
   m_shader.link();

   m_vertices << QVector3D(1, 0, -2)
              << QVector3D(0, 1, -2)
              << QVector3D(-1, 0, -2);
}

void GLCanvas::resizeGL(int width, int height)
{
   // Avoid a divide-by-zero situation:
   if (height == 0)
   {
      height = 1;
   }

   m_projectionMatrix.setToIdentity();
   m_projectionMatrix.perspective(60.0, (float) width / (float) height, 0.001, 1000);

   glViewport(0, 0, width, height);
}

void GLCanvas::paintGL()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   QMatrix4x4 modelMatrix;
   QMatrix4x4 viewMatrix;

   m_shader.bind();
   m_shader.setUniformValue("mvpMatrix", m_projectionMatrix * viewMatrix * modelMatrix);
   m_shader.setUniformValue("color", QColor(Qt::white));
   m_shader.setAttributeArray("vertex", m_vertices.constData());
   m_shader.enableAttributeArray("vertex");

   glDrawArrays(GL_TRIANGLES, 0, m_vertices.size());

   m_shader.disableAttributeArray("vertex");
   m_shader.release();
}

