#ifndef MOCKVIEW_H
#define MOCKVIEW_H

#include <HID/gamepad.h>
#include <Viewport/glCanvas.h>
#include <Windows/baseView.h>

class MockView : public BaseView
{
  public:
    MockView(Controller& controller, QWidget* /*parent*/ = nullptr) : m_controller{ controller }
    {
    }

    /**
     * @brief Show
     */
    void Show(){};

    /**
     * @brief GetWindowHandle
     */
    QWindow* GetWindowHandle()
    {
        return nullptr;
    }

    /**
     * @brief Sets the field of view.
     *
     * @note This function will update both the UI as well as the backing value.
     *
     * @param[in] fieldOfView     The new value to set the field of view to.
     */
    void SetFieldOfViewSlider(int /*fieldOfView*/)
    {
    }

    /**
     * @brief Sets the camera movement speed.
     *
     * @note This function will update both the UI as well as the backing value.
     *
     * @param[in] speed           The new value to set the camera's speed to.
     */
    void SetCameraSpeedSpinner(double /*speed*/)
    {
    }

    /**
     * @brief Sets a temporary message in the status bar.
     *
     * @param[in] message         The message to display.
     * @param[in] timeout         Duration of the message in milliseconds.
     */
    void SetStatusBarMessage(const std::wstring& /*message*/, int /*timeout*/ = 0)
    {
    }

    /**
     * @brief ReloadVisualization
     */
    void ReloadVisualization()
    {
    }

    /**
     * @returns True if the frame time readout should be shown in the titlebar.
     */
    bool ShouldShowFrameTime() const
    {
        return false;
    }

    /**
     * @returns The current search query.
     */
    std::wstring GetSearchQuery() const
    {
        return {};
    }

    /**
     * @returns A reference to the model controller for the treemap visualization.
     */
    Controller& GetController()
    {
        return m_controller;
    }

    /**
     * @returns A reference to the OpenGL canvas.
     */
    GLCanvas& GetCanvas()
    {
        static GLCanvas canvas{ m_controller };
        return canvas;
    }

    /**
     * @returns A reference to the gamepad instance.
     */
    Gamepad& GetGamepad()
    {
        static Gamepad gamepad;
        return gamepad;
    }

    /**
     * @brief AskUserToLimitFileSize
     *
     * @param numberOfFilesScanned
     * @param parameters
     *
     * @return
     */
    bool AskUserToLimitFileSize(
        std::uintmax_t /*numberOfFilesScanned*/, Settings::VisualizationParameters /*parameters*/)
    {
        return false;
    }

    /**
     * @brief DisplayErrorDialog
     *
     * @param message
     */
    void DisplayErrorDialog(std::string_view /*message*/)
    {
    }

    /**
     * @brief SetWaitCursor
     */
    void SetWaitCursor()
    {
    }

    /**
     * @brief RestoreDefaultCursor
     */
    void RestoreDefaultCursor()
    {
    }

    /**
     * @brief OnScanStarted
     */
    void OnScanStarted()
    {
    }

    /**
     * @brief OnScanCompleted
     */
    void OnScanCompleted()
    {
    }

    /**
     * @brief GetTaskbarButton
     * @return
     */
    std::shared_ptr<BaseTaskbarButton> GetTaskbarButton()
    {
        return nullptr;
    }

  private:
    Controller& m_controller;
};

#endif // MOCKVIEW_H
