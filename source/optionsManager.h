#ifndef OPTIONSMANAGER_H
#define OPTIONSMANAGER_H

#include <QObject>

/**
 * @brief The OptionsManager class
 */
class OptionsManager : public QObject
{
   Q_OBJECT;

   public slots:

      /**
       * @brief Should be called when the camera's movement speed changes.
       *
       * @param[in] newSpeed        The new speed.
       */
      void OnCameraMovementSpeedChanged(const double newSpeed);

      /**
       * @brief Should be called when the mouse's movement sensitivity changes.
       *
       * @param[in] newSensitivity  The new sensitivity value.
       */
      void OnMouseSensitivityChanged(const double newSensitivity);

      /**
       * @brief Should be called when the scene's minimum ambient lighting changes.
       *
       * @param[in] newCoefficient  The new ambient lighting coefficient.
       */
      void OnAmbientCoefficientChanged(const double newCoefficient);

      /**
       * @brief Should be called when the point light's attentuation changes.
       *
       * @param[in] newAttenuation  The new attenuation factor.
       */
      void OnAttenuationChanged(const double newAttenuation);

      /**
       * @brief Should be called when the block material shininess changes.
       *
       * @param[in] newShininess    The new shininess value.
       */
      void OnShininessChanged(const double newShininess);

      /**
       * @brief Sets whether the XBox controller is to be used.
       *
       * @param[in] useController   Pass in true to enable the use of the XBox controller.
       */
      void OnUseXBoxControllerStateChanged(const bool useController);

      /**
       * @brief Updates the attachment of the primary light to the camera.
       *
       * @param[in] attached        Pass in true to attach light number one to the camera.
       */
      void OnAttachLightToCameraStateChanged(const bool attached);

      /**
       * @brief Handles changes in the field of view.
       *
       * @param[in] fieldOfView     Field of view in degrees.
       */
      void OnFieldOfViewChanged(int fieldOfView);

   public:

      double m_cameraMovementSpeed{ 0.25 };
      double m_mouseSensitivity{ 0.20 };

      float m_ambientCoefficient{ 0.1f };
      float m_lightAttenuationFactor{ 0.005f };
      float m_materialShininess{ 80.0f };

      int m_fieldOfView{ 45 };

      bool m_isXboxControllerConnected{ false };
      bool m_useXBoxController{ false };
      bool m_isLightAttachedToCamera{ true };
};

#endif // OPTIONSMANAGER_H
