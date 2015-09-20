#ifndef SCENEASSET_H
#define SCENEASSET_H

#include "../../camera.h"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

/**
 * @brief The SceneAsset class is an abstract base class that can be used to simplify the management
 * and rendering of assets in the scene.
 */
class SceneAsset
{
   public:
      explicit SceneAsset();
      ~SceneAsset();

      virtual bool PrepareVertexBuffers(const Camera& camera) = 0;
      virtual bool PrepareColorBuffers(const Camera& camera) = 0;

      bool ClearBuffers();

      virtual bool LoadShaders(const QString& vertexShaderName, const QString& fragmentShaderName);

      QOpenGLShaderProgram& GetVertexShader();

      virtual bool Render(const Camera& camera) = 0;

      bool IsAssetLoaded() const;

   protected:
      QOpenGLBuffer m_vertexBuffer;
      QOpenGLBuffer m_colorBuffer;

      QOpenGLShaderProgram m_shader;

      QOpenGLVertexArrayObject m_VAO;

      QVector<QVector3D> m_rawVertices;
      QVector<QVector3D> m_rawColors;
};

#endif // SCENEASSET_H
