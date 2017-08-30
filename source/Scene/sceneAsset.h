#ifndef SCENEASSET_H
#define SCENEASSET_H

#include "../DataStructs/light.h"
#include "../optionsManager.h"
#include "../Viewport/camera.h"
#include "../Visualizations/visualization.h"

#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

struct VizFile;

template<typename DataType>
class TreeNode;

/**
 * @brief The SceneAsset class is an abstract base class that can be used to simplify the management
 * and rendering of assets in the scene.
 */
class SceneAsset
{
   public:

      enum struct UpdateAction : short
      {
         SELECT = 0,
         DESELECT
      };

      explicit SceneAsset(QOpenGLExtraFunctions& device);

      virtual ~SceneAsset();

      /**
       * @brief Loads the vertex and color data into the OpenGL buffers. Use
       * SceneAsset::SetVertexData() and SceneAsset::SetColorData() to set the data before calling
       * this function.
       *
       * @param[in] camera          The camera associated with the OpenGL context.
       *
       * @returns True if the operation succeeded.
       */
      virtual bool Initialize() = 0;

      /**
       * @brief Clears the OpenGL buffer objects that contain the vertex and color data.
       *
       * @returns True if the operation succeeded.
       */
      bool ClearBuffers();

      /**
       * @brief Loads the vertex and fragment shaders.
       *
       * @returns True if the operation succeeded.
       */
      virtual bool LoadShaders() = 0;

      /**
       * @brief Performs the necessary actions to render the asset in question to the OpenGL canvas.
       *
       * @param[in] camera          The camera associated with the OpenGL context.
       * @param[in] light           The light source for the scene.
       * @param[in] settings        Any additional settings relevant to rendering.
       *
       * @returns True if the operation succeeded.
       */
      virtual bool Render(
         const Camera& camera,
         const std::vector<Light>& light,
         const OptionsManager& settings) = 0;

      /**
       * @brief Determines whether the scene asset has been loaded.
       *
       * @returns True if the OpenGL buffers have been properly populated.
       */
      virtual bool IsAssetLoaded() const;

      /**
       * @brief Performs all actions necessary to update the data in the OpenGL buffers.
       *
       * @param[in] camera          The camera associated with the OpenGL context.
       *
       * @returns True if the operation succeeded.
       */
      virtual bool Reload() = 0;

      /**
       * @brief Sets the vertex data associated with the asset in question.
       *
       * @param[in] data            The asset vertices and normals.
       */
      void SetVertexCoordinates(QVector<QVector3D>&& positionData);

      /**
       * @brief Set the color data associated with the asset in question.
       *
       * @param[in] data            The vertex colors.
       */
      void SetVertexColors(QVector<QVector3D>&& colorData);

      /**
       * @brief Adds more vertex positions the existing collection. Make sure to also insert an
       * equal number of vertex colors by calling AddVertexColors.
       *
       * @param[in] positionData    The new vertices to be added.
       */
      void AddVertexCoordinates(QVector<QVector3D>&& positionData);

      /**
       * @brief Adds more vertex colors the existing collection. Make sure to also insert an
       * equal number of vertex positions by calling AddVertexCoordinates.
       *
       * @param[in] positionData    The new vertices to be added.
       */
      void AddVertexColors(QVector<QVector3D>&& colorData);

      /**
       * @brief Retrieves vizualization vertex count.
       *
       * @returns The count of vertices used to represent the asset.
       */
      unsigned int GetVertexCount() const;

      /**
       * @brief Retrieves vizualization color count.
       *
       * @returns The count of color entries used to present the asset's color.
       */
      unsigned int GetColorCount() const;

      /**
       * @brief Updates the portion of the VBO associated with the specified TreeNode.
       *
       * @param[in] node            The TreeNode whose visualization should be updated.
       * @param[in] action          The type of update to perform on the target VBO segment.
       */
      virtual void UpdateVBO(
         const Tree<VizFile>::Node& node,
         UpdateAction action,
         const VisualizationParameters& options);

   protected:

      /**
       * @brief Helper function to compile and load the specified OpenGL shaders.
       *
       * @param[in] vertexShaderName      Filename of the vertex shader.
       * @param[in] fragmentShaderName    Filename of the fragment shader.
       *
       * @returns True if compilation and loading suceeded.
       */
      bool LoadShaders(
         const QString& vertexShaderName,
         const QString& fragmentShaderName);

      QOpenGLBuffer m_vertexBuffer;
      QOpenGLBuffer m_colorBuffer;

      QOpenGLShaderProgram m_mainShader;

      QOpenGLVertexArrayObject m_VAO;

      QVector<QVector3D> m_rawVertices;
      QVector<QVector3D> m_rawColors;

      QOpenGLExtraFunctions& m_graphicsDevice;
};

#endif // SCENEASSET_H
