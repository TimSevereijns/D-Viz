#ifndef MOCKVIEW_H
#define MOCKVIEW_H

#include <View/baseView.h>

#include <QtTest>

#include "Utilities/trompeloeilAdapter.h"

class BaseTaskbarButton;
class Controller;
class Gamepad;
class GLCanvas;
class QWidget;

namespace Settings
{
    class VisualizationOptions;
}

class MockView : public BaseView
{
  public:
    MockView(Controller& controller, QWidget* /*parent*/ = nullptr) : m_controller{ controller }
    {
    }

    MAKE_MOCK0(Show, void(), override);
    MAKE_MOCK0(GetWindowHandle, QWindow*(), override);
    MAKE_MOCK1(SetFieldOfViewSlider, void(int), override);
    MAKE_MOCK1(SetCameraSpeedSpinner, void(double), override);
    MAKE_MOCK2(SetStatusBarMessage, void(const std::string&, int), override);
    MAKE_MOCK0(ReloadVisualization, void(), override);
    MAKE_CONST_MOCK0(ShouldShowFrameTime, bool(), override);
    MAKE_CONST_MOCK0(GetSearchQuery, std::string(), override);
    MAKE_MOCK0(GetController, Controller&(), override);
    MAKE_MOCK0(GetCanvas, GLCanvas&(), override);
    MAKE_MOCK0(GetGamepad, Gamepad&(), override);
    MAKE_MOCK1(AskUserToLimitFileSize, bool(std::uintmax_t), override);
    MAKE_MOCK1(DisplayInfoDialog, void(std::string_view), override);
    MAKE_MOCK1(DisplayErrorDialog, void(std::string_view), override);
    MAKE_MOCK0(SetWaitCursor, void(), override);
    MAKE_MOCK0(RestoreDefaultCursor, void(), override);
    MAKE_MOCK0(OnScanStarted, void(), override);
    MAKE_MOCK0(OnScanCompleted, void(), override);
    MAKE_MOCK0(GetTaskbarButton, std::shared_ptr<BaseTaskbarButton>(), override);

  private:
    Controller& m_controller;
};

#endif // MOCKVIEW_H
