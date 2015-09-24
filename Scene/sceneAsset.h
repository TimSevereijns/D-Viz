#ifndef SCENEASSET_H
#define SCENEASSET_H

#include "../camera.h"
#include "../DataStructs/light.h"
#include "../optionsManager.h"

#include <QOpenGLBuffer>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

/**
 * @brief The SceneAsset class is an abstract base class that can be used to simplify the management
 * and rendering of assets in the scene.
 */
class SceneAsset : protected QOpenGLFunctions
{
   public:
      explicit SceneAsset();
      virtual ~SceneAsset();

      virtual bool PrepareVertexBuffers(const Camera& camera) = 0;
      virtual bool PrepareColorBuffers(const Camera& camera) = 0;

      bool ClearBuffers();

      virtual bool LoadShaders() = 0;

      QOpenGLShaderProgram& GetVertexShader();

      virtual bool Render(const Camera& camera, const Light& light, bool isVizualizationLoaded,
                          const OptionsManager& settings) = 0;

      bool IsAssetLoaded() const;

      virtual bool Reload(const Camera& camera) = 0;

      void SetVertexData(QVector<QVector3D>&& data);

      void SetColorData(QVector<QVector3D>&& data);

   protected:
      bool LoadShaders(const QString& vertexShaderName, const QString& fragmentShaderName);

      QOpenGLBuffer m_vertexBuffer;
      QOpenGLBuffer m_colorBuffer;

      QOpenGLShaderProgram m_shader;

      QOpenGLVertexArrayObject m_VAO;

      QVector<QVector3D> m_rawVertices;
      QVector<QVector3D> m_rawColors;
};

#endif // SCENEASSET_H
