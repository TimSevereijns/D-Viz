#include "graphicsDevice.h"

GraphicsDevice::GraphicsDevice()
   : QOpenGLFunctions()
{
   initializeOpenGLFunctions();
}

