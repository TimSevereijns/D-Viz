#include "graphicsDevice.h"

GraphicsDevice::GraphicsDevice() :
   QOpenGLFunctions_4_5_Core{ }
{
   initializeOpenGLFunctions();
}
