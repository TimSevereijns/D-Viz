#include "graphicsDevice.h"

GraphicsDevice::GraphicsDevice() :
   QOpenGLFunctions_3_3_Core{ }
{
   initializeOpenGLFunctions();
}
