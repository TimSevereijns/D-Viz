#ifndef BASEVIEW_H
#define BASEVIEW_H

#include <memory>
#include <string>
#include <string_view>

#include "Settings/settings.h"

class Controller;
class XboxGamepad;
class GLCanvas;
class QWidget;
class QWindow;
class QObject;

class BaseTaskbarButton
{
  public:
    virtual ~BaseTaskbarButton() noexcept = default;

    /**
     * @brief Sets the owner of the taskbar button.
     */
    virtual void SetWindow(QObject* /*window*/)
    {
    }

    /**
     * @brief Hides the progress overlay.
     */
    virtual void HideProgress()
    {
    }

    /**
     * @brief Resetse the progress overlay.
     */
    virtual void ResetProgress()
    {
    }

    /**
     * @brief Sets the normalized progress value.
     */
    virtual void SetValue(int)
    {
    }

    /**
     * @brief Sets the minimum for the progress range.
     */
    virtual void SetMinimum(int)
    {
    }

    /**
     * @brief Sets the maximum for the progress range.
     */
    virtual void SetMaximum(int)
    {
    }

    /**
     * @brief Sets the visibility of the progress overlay.
     */
    virtual void SetVisible(bool)
    {
    }
};

class BaseView
{
  public:
    BaseView(QWidget* /*parent*/ = nullptr)
    {
    }

    virtual ~BaseView() noexcept = default;

    /**
     * @brief Show
     */
    virtual void Show() = 0;

    /**
     * @returns The window handle.
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
    virtual void SetStatusBarMessage(const std::string& message, int timeout = 0) = 0;

    /**
     * @brief Reloads the visualization.
     */
    virtual void ReloadVisualization() = 0;

    /**
     * @returns True if the frame time readout should be shown in the titlebar.
     */
    virtual bool ShouldShowFrameTime() const = 0;

    /**
     * @returns The current search query.
     */
    virtual std::string GetSearchQuery() const = 0;

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
    virtual XboxGamepad& GetGamepad() = 0;

    /**
     * @brief Asks the user to consider limiting the number of files to display, and then saves the
     * response to the relevant setting.
     *
     * @param[in] numberOfFilesScanned  The number of files scanned.
     *
     * @returns True if the user opted to change the parameters.
     */
    virtual bool AskUserToLimitFileSize(std::uintmax_t numberOfFilesScanned) = 0;

    /**
     * @brief Displays an info dialog.
     *
     * @param[in] message           The message to be displayed in the dialog.
     */
    virtual void DisplayInfoDialog(std::string_view message) = 0;

    /**
     * @brief Displays an error dialog.
     *
     * @param[in] message           The message to be displayed in the dialog.
     */
    virtual void DisplayErrorDialog(std::string_view message) = 0;

    /**
     * @brief Sets the cursor to a wait symbol.
     */
    virtual void SetWaitCursor() = 0;

    /**
     * @brief Restores the cursor back to the default symbol.
     */
    virtual void RestoreDefaultCursor() = 0;

    /**
     * @brief Called when a scan starts.
     */
    virtual void OnScanStarted() = 0;

    /**
     * @brief Called when a scan completes.
     */
    virtual void OnScanCompleted() = 0;

    /**
     * @returns The platform-specific taskbar button.
     */
    virtual std::shared_ptr<BaseTaskbarButton> GetTaskbarButton() = 0;
};

#endif // BASEVIEW_H
