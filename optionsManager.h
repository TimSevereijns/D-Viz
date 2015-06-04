#ifndef OPTIONSMANAGER_H
#define OPTIONSMANAGER_H

#include <QObject>

/**
 * @brief The OptionsManager class
 */
class OptionsManager : public QObject
{
   Q_OBJECT;

   public:
      OptionsManager();
      ~OptionsManager();

   public slots:
      /**
       * @brief OnCameraMovementSpeedChanged should be called when the camera's movement speed
       * changes.
       *
       * @param[in] newSpeed        The new speed.
       */
      void OnCameraMovementSpeedChanged(const double newSpeed);

      /**
       * @brief OnMouseSensitivityChanged should be called when the mouse's movement sensitivity
       * changes.
       *
       * @param[in] newSensitivity  The new sensitivity value.
       */
      void OnMouseSensitivityChanged(const double newSensitivity);

      /**
       * @brief OnAmbientCoefficientChanged should be called when the scene's minimum ambient
       * lighting changes.
       *
       * @param[in] newCoefficient  The new ambient lighting coefficient.
       */
      void OnAmbientCoefficientChanged(const double newCoefficient);

      /**
       * @brief OnAttenuationChanged should be called when the point light's attentuation changes.
       *
       * @param[in] newAttenuation  The new attenuation factor.
       */
      void OnAttenuationChanged(const double newAttenuation);

      /**
       * @brief OnShininessChanged should be called when the block material shininess changes.
       *
       * @param[in] newShininess    The new shininess value.
       */
      void OnShininessChanged(const double newShininess);

      /**
       * @brief OnUseXBoxControllerStateChanged sets whether the XBox controller is to be used.
       *
       * @param[in] useController   Pass in true to enable the use of the XBox controller.
       */
      void OnUseXBoxControllerStateChanged(const bool useController);

      /**
       * @brief OnAttachLightToCameraStateChanged
       * @param attached
       */
      void OnAttachLightToCameraStateChanged(const bool attached);

      /**
       * @brief OnFieldOfViewChanged
       * @param fieldOfView
       *
       */
      void OnFieldOfViewChanged(int fieldOfView);

   public:
      double m_cameraMovementSpeed;
      double m_mouseSensitivity;

      float m_ambientCoefficient;
      float m_lightAttenuationFactor;
      float m_materialShininess;

      int m_fieldOfView;

      bool m_isXboxControllerConnected;
      bool m_useXBoxController;
      bool m_isLightAttachedToCamera;
};

#endif // OPTIONSMANAGER_H
