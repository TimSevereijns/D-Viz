#ifndef SCENEASSET_H
#define SCENEASSET_H

#include "../camera.h"
#include "../DataStructs/light.h"
#include "../optionsManager.h"

#include "graphicsDevice.h"

#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

/**
 * @brief The SceneAsset class is an abstract base class that can be used to simplify the management
 * and rendering of assets in the scene.
 */
class SceneAsset
{
   public:
      explicit SceneAsset(GraphicsDevice& device);
      virtual ~SceneAsset();

      virtual bool PrepareVertexBuffers(const Camera& camera) = 0;
      virtual bool PrepareColorBuffers(const Camera& camera) = 0;

      bool ClearBuffers();

      virtual bool LoadShaders() = 0;

      QOpenGLShaderProgram& GetVertexShader();

      virtual bool Render(const Camera& camera, const Light& light,
         const OptionsManager& settings) = 0;

      bool IsAssetLoaded() const;

      virtual bool Reload(const Camera& camera) = 0;

      void SetVertexData(QVector<QVector3D>&& data);

      void SetColorData(QVector<QVector3D>&& data);

      unsigned int GetVertexCount() const;
      unsigned int GetColorCount() const;

   protected:
      bool LoadShaders(const QString& vertexShaderName, const QString& fragmentShaderName);

      QOpenGLBuffer m_vertexBuffer;
      QOpenGLBuffer m_colorBuffer;

      QOpenGLShaderProgram m_shader;

      QOpenGLVertexArrayObject m_VAO;

      QVector<QVector3D> m_rawVertices;
      QVector<QVector3D> m_rawColors;

      GraphicsDevice& m_graphicsDevice;
};

#endif // SCENEASSET_H
