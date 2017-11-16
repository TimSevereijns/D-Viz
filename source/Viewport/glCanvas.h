#ifndef GLCANVAS_H
#define GLCANVAS_H

#include "camera.h"
#include "controller.h"
#include "DataStructs/light.h"
#include "DriveScanner/driveScanner.h"
#include "HID/keyboardManager.h"
#include "settingsManager.h"
#include "Scene/baseAsset.h"
#include "Scene/crosshairAsset.h"
#include "Scene/debuggingRayAsset.h"
#include "Scene/gridAsset.h"
#include "Scene/lightMarkerAsset.h"
#include "Scene/originMarkerAsset.h"
#include "Scene/treemapAsset.h"
#include "Visualizations/visualization.h"
#include "Windows/mainWindow.h"

#include <chrono>
#include <deque>
#include <memory>

#include <QOpenGLWidget>
#include <QTimer>
#include <QVector3D>

namespace Asset
{
   namespace Tag
   {
      struct Base
      {
         using AssetType = void;
         virtual int GetID() const noexcept { return 0; }
      };

      struct OriginMarker final : Base
      {
         using AssetType = Asset::OriginMarker;
         int GetID() const noexcept override { return 1; }
      };

      struct Grid final : Base
      {
         using AssetType = Asset::Grid;
         int GetID() const noexcept override { return 2; }
      };

      struct Crosshair final : Base
      {
          using AssetType = Asset::Crosshair;
          int GetID() const noexcept override { return 3; }
      };

      struct Treemap final : Base
      {
         using AssetType = Asset::Treemap;
         int GetID() const noexcept override { return 4; }
      };

      struct LightMarker final : Base
      {
         using AssetType = Asset::LightMarker;
         int GetID() const noexcept override { return 5; }
      };
   }
}

/**
 * @brief The GLCanvas class represents the canvas object on which the visualization is to be drawn.
 *
 * This class contains the central rendering code that invokes the render functions on the
 * individual scene assets that make up the entire visualization. Camera movement and scene
 * interaction are also handled by this class.
 */
class GLCanvas final : public QOpenGLWidget
{
   Q_OBJECT

   public:

      /**
       * @brief Constructs a new GLCanvas object.
       *
       * @param[in] controller      The Controller object that'll manage the visual representation
       *                            of the underlying treemap.
       * @param[in] parent          A pointer to the canvas's parent window.
       */
      GLCanvas(
         Controller& controller,
         QWidget* parent = nullptr);

      /**
       * @brief Reloads the current visualization.
       *
       * @param[in] parameters      @see VisualizationParameters
       */
      void ReloadVisualization();

      /**
       * @brief Applies the currently active color scheme, as set via the Settings::Manager.
       */
      void ApplyColorScheme();

      /**
       * @brief Sets the current field of view for the camera.
       *
       * @param[in] fieldOfView     The new field of view.
       */
      void SetFieldOfView(const float fieldOfView);

      /**
       * @brief Paints the selected node.
       *
       * @param[in] node            The node whose visual representation is to be repainted.
       */
      inline void SelectNode(const Tree<VizFile>::Node* const node);

      /**
       * @brief Restores the color of the selected node back to its unselected state.
       */
      void RestoreSelectedNode();

      /**
       * @brief HighlightSelectedNodes
       */
      void HighlightNodes(std::vector<const Tree<VizFile>::Node*>& nodes);

      /**
       * @brief Returns any highlighted nodes back to their unhighlighted colors.
       */
      void RestoreHighlightedNodes(std::vector<const Tree<VizFile>::Node*>& nodes);

      /**
       * @brief ToggleRenderState
       *
       * @param desiredState
       */
      template<typename TagType>
      void ToggleAssetVisibility(bool desiredState) const noexcept
      {
         auto* const asset = GetAsset<TagType>();

         if (desiredState == true)
         {
            asset->Show();
         }
         else
         {
            asset->Hide();
         }
      }

   protected:

      void initializeGL() override;
      void resizeGL(int width, int height) override;
      void paintGL() override;

      void keyPressEvent(QKeyEvent* event) override;
      void keyReleaseEvent(QKeyEvent* event) override;

      void mouseReleaseEvent(QMouseEvent* event) override;
      void mousePressEvent(QMouseEvent* event) override;
      void mouseMoveEvent(QMouseEvent* event) override;
      void wheelEvent(QWheelEvent* event) override;

   private slots:

      /**
       * @brief Handles keyboard, mouse, and gamepad input, and also updates the OpenGL canvas.
       */
      void RunMainLoop();

   private:

      /**
       * @brief Handle Input
       */
      void HandleUserInput();

      /**
       * @brief Records the elapsed frame time.
       */
      void UpdateFrameTime(const std::chrono::microseconds& elapsedTime);

      /**
       * @brief Generates and displays the context menu.
       *
       * @param[in] point           The location at which to place the top left corner of the menu.
       */
      void ShowContextMenu(const QPoint& point);

      /**
       * @brief HandleKeyboardInput
       *
       * @param[in] elapsedTime
       */
      void HandleKeyboardInput(const std::chrono::milliseconds& elapsedTime);

      /**
       * @brief Handles the input from the gamepad controller.
       *
       * @param[in] elapsedTime
       */
      void HandleGamepadInput(const std::chrono::milliseconds& elapsedTime);

      /**
       * @brief HandleGamepadKeyInput
       *
       * @param gamepad
       * @param elapsedTime
       */
      void HandleGamepadButtonInput(
         const Gamepad& gamepad,
         const std::chrono::milliseconds& elapsedTime);

      /**
       * @brief Handles left and right trigger input.
       *
       * @param[in] gamepad
       */
      void HandleGamepadTriggerInput(const Gamepad& gamepad);

      /**
       * @brief Handles thumb stick inputs.
       *
       * @param[in] gamepad.
       */
      void HandleGamepadThumbstickInput(const Gamepad& gamepad);

      /**
       * @brief Compiles and loads the OpenGL shader program for the visualization.
       */
      void PrepareVisualizationShaderProgram();

      /**
       * @brief Compiles and loads the OpenGL shader program for the origin marker.
       */
      void PrepareOriginMarkerShaderProgram();

      /**
       * @brief Initializes the vertex buffer for the visualization.
       */
      void PrepareVisualizationVertexBuffers();

      /**
       * @brief Initializes the vertex buffer for the origin marker.
       */
      void PrepareOriginMarkerVertexBuffers();

      /**
       * @brief SelectNodeViaRay
       */
      void SelectNodeViaRay(const QPoint& rayOrigin);

      /**
       * @brief Helper function that turns scene asset retrieval into a simple one-liner.
       *
       * @tparam RequestedAsset     The tag specifying the type of the asset that is to be
       *                            retrieved. Note that we're implicitly assuming that we'll only
       *                            store one instance of each type.
       */
      template<typename RequestedAsset>
      typename RequestedAsset::AssetType* GetAsset() const noexcept
      {
         const auto itr = std::find_if(std::begin(m_sceneAssets), std::end(m_sceneAssets),
           [targetID = RequestedAsset{ }.GetID()] (const auto& tagAndAsset) noexcept
         {
            return tagAndAsset.tag->GetID() == targetID;
         });

         if (itr == std::end(m_sceneAssets))
         {
            assert(false);
            return nullptr;
         }

         return static_cast<typename RequestedAsset::AssetType*>(itr->asset.get());
      }

      /**
       * @brief Helper function that turns scene asset registration into a simple one-liner.
       *
       * @tparam AssetTag           The tag specifying the type of asset to register. Note that
       *                            we're implicitly assuming that we'll only store one instance of
       *                            each type.
       */
      template<typename AssetTag>
      void RegisterAsset()
      {
         m_sceneAssets.emplace_back(TagAndAsset
         {
            std::make_unique<AssetTag>(),
            std::make_unique<typename AssetTag::AssetType>(m_graphicsDevice)
         });
      }

      bool m_isPaintingSuspended{ false };
      bool m_isVisualizationLoaded{ false };
      bool m_isLeftTriggerDown{ false };
      bool m_isRightTriggerDown{ false };
      bool m_isLeftMouseButtonDown{ false };
      bool m_isCursorHidden{ false };

      Controller& m_controller;

      MainWindow& m_mainWindow;

      QOpenGLExtraFunctions m_graphicsDevice;

      QTimer m_frameRedrawTimer{ nullptr };

      std::chrono::system_clock::time_point m_lastFrameDrawTime
      {
         std::chrono::system_clock::now()
      };

      std::chrono::system_clock::time_point m_lastCameraPositionUpdatelTime
      {
         std::chrono::system_clock::now()
      };

      std::chrono::system_clock::time_point m_startOfMouseLookEvent
      {
         std::chrono::system_clock::now()
      };

      std::vector<Light> m_lights
      {
         Light{ },
         Light{ QVector3D{ 0.0f, 80.0f, 0.0f } },
         Light{ QVector3D{ 0.0f, 80.0f, -VisualizationModel::ROOT_BLOCK_DEPTH } },
         Light{ QVector3D{ VisualizationModel::ROOT_BLOCK_WIDTH, 80.0f, 0.0f } },
         Light{ QVector3D{ VisualizationModel::ROOT_BLOCK_WIDTH, 80.0f, -VisualizationModel::ROOT_BLOCK_DEPTH } }
      };

      Camera m_camera;

      KeyboardManager m_keyboardManager;

      QMatrix4x4 m_projectionMatrix;

      QPoint m_lastMousePosition;

      struct TagAndAsset
      {
         std::unique_ptr<Asset::Tag::Base> tag;
         std::unique_ptr<Asset::Base> asset;
      };

      // @note Using an unsorted, linear container to store and retrieve assets is likely to
      // outperform std::unordered_map for a small number of assets. Should the asset count ever
      // grow past, say, 30 assets, then a std::unordered_map might start to make more sense.
      std::vector<TagAndAsset> m_sceneAssets;

      std::deque<int> m_frameTimeDeque;
};

#endif // GLCANVAS_H
