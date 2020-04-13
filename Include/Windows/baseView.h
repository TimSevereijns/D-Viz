#ifndef BASEVIEW_H
#define BASEVIEW_H

#include <memory>
#include <string>
#include <string_view>

#include "Settings/settings.h"

class Controller;
class Gamepad;
class GLCanvas;
class QWindow;
class QObject;

class BaseTaskbarButton
{
  public:
    virtual ~BaseTaskbarButton() noexcept = default;

    virtual void SetWindow(QObject* /*window*/)
    {
    }

    virtual void HideProgress()
    {
    }

    virtual void ResetProgress()
    {
    }

    virtual void SetValue(int)
    {
    }

    virtual void SetMinimum(int)
    {
    }

    virtual void SetMaximum(int)
    {
    }

    virtual void SetVisible(bool)
    {
    }
};

class BaseView
{
  public:
    virtual ~BaseView() noexcept = default;

    /**
     * @brief Show
     */
    virtual void Show() = 0;

    /**
     * @brief GetWindowHandle
     */
    virtual QWindow* GetWindowHandle() = 0;

    /**
     * @brief Sets the field of view.
     *
     * @note This function will update both the UI as well as the backing value.
     *
     * @param[in] fieldOfView     The new value to set the field of view to.
     */
    virtual void SetFieldOfViewSlider(int fieldOfView) = 0;

    /**
     * @brief Sets the camera movement speed.
     *
     * @note This function will update both the UI as well as the backing value.
     *
     * @param[in] speed           The new value to set the camera's speed to.
     */
    virtual void SetCameraSpeedSpinner(double speed) = 0;

    /**
     * @brief Sets a temporary message in the status bar.
     *
     * @param[in] message         The message to display.
     * @param[in] timeout         Duration of the message in milliseconds.
     */
    virtual void SetStatusBarMessage(const std::wstring& message, int timeout = 0) = 0;

    /**
     * @brief ReloadVisualization
     */
    virtual void ReloadVisualization() = 0;

    /**
     * @returns True if the frame time readout should be shown in the titlebar.
     */
    virtual bool ShouldShowFrameTime() const = 0;

    /**
     * @returns The current search query.
     */
    virtual std::wstring GetSearchQuery() const = 0;

    /**
     * @returns A reference to the model controller for the treemap visualization.
     */
    virtual Controller& GetController() = 0;

    /**
     * @returns A reference to the OpenGL canvas.
     */
    virtual GLCanvas& GetCanvas() = 0;

    /**
     * @returns A reference to the gamepad instance.
     */
    virtual Gamepad& GetGamepad() = 0;

    /**
     * @brief AskUserToLimitFileSize
     *
     * @param numberOfFilesScanned
     * @param parameters
     *
     * @return
     */
    virtual bool AskUserToLimitFileSize(
        std::uintmax_t numberOfFilesScanned, Settings::VisualizationParameters parameters) = 0;

    /**
     * @brief DisplayErrorDialog
     *
     * @param message
     */
    virtual void DisplayErrorDialog(std::string_view message) = 0;

    /**
     * @brief SetWaitCursor
     */
    virtual void SetWaitCursor() = 0;

    /**
     * @brief RestoreDefaultCursor
     */
    virtual void RestoreDefaultCursor() = 0;

    /**
     * @brief OnScanStarted
     */
    virtual void OnScanStarted() = 0;

    /**
     * @brief OnScanCompleted
     */
    virtual void OnScanCompleted() = 0;

    /**
     * @brief GetTaskbarButton
     * @return
     */
    virtual std::shared_ptr<BaseTaskbarButton> GetTaskbarButton() = 0;
};

#endif // BASEVIEW_H
