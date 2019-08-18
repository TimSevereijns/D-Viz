#ifndef BASEASSET_H
#define BASEASSET_H

#include "Scene/light.h"
#include "Settings/settingsManager.h"
#include "Viewport/camera.h"
#include "Visualizations/visualization.h"

#include <QOpenGLBuffer>
#include <QOpenGLExtraFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLVertexArrayObject>

struct VizBlock;

template <typename DataType> class TreeNode;

class Controller;

namespace Assets
{
    /**
     * @brief The Asset::Base class is an abstract base class that can be used to simplify the
     * management and rendering of assets in the scene.
     */
    class AssetBase
    {
      public:
        /**
         * @brief Constructs a new instance of the Asset::Base class.
         *
         * @param[in] controller            Reference to the controller class.
         * @param[in] openGL                The OpenGL function bindings.
         */
        AssetBase(const Controller& controller, QOpenGLExtraFunctions& openGL);

        AssetBase(const AssetBase& other) = delete;
        AssetBase& operator=(const AssetBase& other) = delete;

        AssetBase(AssetBase&& other) = default;
        AssetBase& operator=(AssetBase&& other) = default;

        virtual ~AssetBase() = default;

        /**
         * @brief Loads the vertex and color data into the OpenGL buffers. Use
         * Asset::Base::SetVertexData() and Asset::Base::SetColorData() to set the data before
         * calling
         * this function.
         *
         * @param[in] camera          The camera associated with the OpenGL context.
         */
        virtual void Initialize() = 0;

        /**
         * @brief Clears the OpenGL buffer objects that contain the vertex and color data.
         */
        void ClearBuffers();

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
         */
        virtual void Render(const Camera& camera, const std::vector<Light>& light) = 0;

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
         */
        virtual void Refresh() = 0;

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
         * @brief Adds more vertex colors the existing collection. Make sure to also insert an equal
         * number of vertex positions by calling AddVertexCoordinates.
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

      protected:
        /**
         * @brief Helper function to compile and load the specified OpenGL shaders.
         *
         * @param[in] vertexShaderName      Filename of the vertex shader.
         * @param[in] fragmentShaderName    Filename of the fragment shader.
         *
         * @returns True if compilation and loading suceeded.
         */
        bool LoadShaders(const QString& vertexShaderName, const QString& fragmentShaderName);

        bool DetermineVisibilityFromPreferences(std::wstring_view assetName);

        QOpenGLBuffer m_vertexBuffer;
        QOpenGLBuffer m_colorBuffer;

        QOpenGLShaderProgram m_mainShader;

        QOpenGLVertexArrayObject m_VAO;

        QVector<QVector3D> m_rawVertices;
        QVector<QVector3D> m_rawColors;

        QOpenGLExtraFunctions& m_openGL;

        const Settings::Manager& m_settingsManager;

        bool m_shouldRender{ true };
    };
} // namespace Assets

#endif // BASEASSET_H
