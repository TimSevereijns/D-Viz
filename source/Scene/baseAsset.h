#ifndef BASEASSET_H
#define BASEASSET_H

#include "../DataStructs/light.h"
#include "../Settings/settingsManager.h"
#include "../Viewport/camera.h"
#include "../Visualizations/visualization.h"

#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

struct VizFile;

template<typename DataType>
class TreeNode;

namespace Asset
{
   enum struct Event : short
   {
      SELECTION = 0,
      DESELECTION
   };

   /**
    * @brief The Asset::Base class is an abstract base class that can be used to simplify the
    * management and rendering of assets in the scene.
    */
   class Base
   {
      public:

         explicit Base(
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

         virtual ~Base() = default;

         /**
          * @brief Loads the vertex and color data into the OpenGL buffers. Use
          * Asset::Base::SetVertexData() and Asset::Base::SetColorData() to set the data before
          * calling this function.
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
          * @brief Performs the necessary actions to render the asset in question to the OpenGL
          * canvas.
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
            const Settings::Manager& settings) = 0;

         /**
          * @brief Determines whether the scene asset has been loaded.
          *
          * @returns True if the OpenGL buffers have been properly populated.
          */
         virtual bool IsAssetLoaded() const;

         /**
          * @brief Performs all actions necessary to update the data in the OpenGL buffers.
          * Implementing and calling this function is especially important if the size of the vertex
          * and color buffers ever changes.
          *
          * @param[in] camera          The camera associated with the OpenGL context.
          *
          * @returns True if the operation succeeded.
          */
         virtual bool Refresh() = 0;

         /**
          * @brief Sets the vertex data associated with the asset in question.
          *
          * @param[in] data            The asset vertices and normals.
          */
         void SetVertexData(QVector<QVector3D>&& data);

         /**
          * @brief Set the color data associated with the asset in question.
          *
          * @param[in] data            The vertex colors.
          */
         void SetColorData(QVector<QVector3D>&& data);

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
          * @brief Specifies that this asset should be drawn with the next invocation of the
          * rendering loop.
          */
         virtual void Show();

         /**
          * @brief Specifies that this asset should not be drawn with the next invocation of the
          * rendering loop.
          *
          * @note Memory intensive assets should consider whether it might be wise to unload vertex
          * and color data buffers if the asset is to remain hidden for a while.
          */
         virtual void Hide();

         /**
          * @brief Updates the portion of the VBO associated with the specified TreeNode.
          *
          * @param[in] node            The TreeNode whose visualization should be updated.
          * @param[in] action          The type of update to perform on the target VBO segment.
          * @param[in] settings        @todo
          */
         virtual void UpdateVBO(
            const Tree<VizFile>::Node& node,
            Event action,
            const Settings::Manager& settings);

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

         QOpenGLShaderProgram m_shader;

         QOpenGLVertexArrayObject m_VAO;

         QVector<QVector3D> m_rawVertices;
         QVector<QVector3D> m_rawColors;

         QOpenGLExtraFunctions& m_openGL;

         bool m_shouldRender{ true };
   };
}

#endif // BASEASSET_H
