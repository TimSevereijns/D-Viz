#ifndef SCENEASSET_H
#define SCENEASSET_H

#include "../Viewport/camera.h"
#include "../DataStructs/light.h"
#include "../optionsManager.h"

#include "../Viewport/graphicsDevice.h"

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

      /**
       * @brief PrepareVertexBuffers loads the vertex data into the OpenGL buffer. Use
       * SceneAsset::SetVertexData() to set the data before calling this function.
       *
       * @param camera              The camera associated with the OpenGL context.
       *
       * @returns true if the operation succeeded.
       */
      virtual bool PrepareVertexBuffers(const Camera& camera) = 0;

      /**
       * @brief PrepareColorBuffers loads the color data into the OpenGL buffer. Use
       * SceneAsset::SetColorData() to set the data before calling this function.
       *
       * @param camera              The camera associated with the OpenGL context.
       *
       * @returns true if the operation succeeded.
       */
      virtual bool PrepareColorBuffers(const Camera& camera) = 0;

      /**
       * @brief ClearBuffers clears the OpenGL buffer objects that contain the vertex and
       * color data.
       *
       * @returns true if the operation succeeded.
       */
      bool ClearBuffers();

      /**
       * @brief LoadShaders loads the vertex and fragment shaders.
       *
       * @returns true if the operation succeeded.
       */
      virtual bool LoadShaders() = 0;

      /**
       * @brief Render performs the necessary actions to render the asset in question to the
       * OpenGL canvas.
       *
       * @param camera              The camera associated with the OpenGL context.
       * @param light               The light source for the scene.
       * @param settings            Any additional settings relevant to rendering.
       *
       * @returns true if the operation succeeded.
       */
      virtual bool Render(const Camera& camera, const Light& light,
         const OptionsManager& settings) = 0;

      /**
       * @brief IsAssetLoaded
       *
       * @returns true if the OpenGL buffers have been properly populated.
       */
      bool IsAssetLoaded() const;

      /**
       * @brief Reload performs all actions necessary to update the data in the OpenGL buffers.
       *
       * @param camera              The camera associated with the OpenGL context.
       *
       * @returns true if the operation succeeded.
       */
      virtual bool Reload(const Camera& camera) = 0;

      /**
       * @brief SetVertexData is the primary function to set the vertex data associated with the
       * asset in question.
       *
       * @param data                The asset vertices and normals.
       */
      void SetVertexData(QVector<QVector3D>&& data);

      /**
       * @brief SetColorData is the primary function to set the color data associated with the
       * asset in question.
       *
       * @param data                The vertex colors.
       */
      void SetColorData(QVector<QVector3D>&& data);

      /**
       * @brief GetVertexCount
       * @returns the count of vertices used to represent the asset.
       */
      unsigned int GetVertexCount() const;

      /**
       * @brief GetColorCount
       * @returns the count of color entries used to present the asset's color.
       */
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
