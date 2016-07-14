#include "graphicsDevice.h"

GraphicsDevice::GraphicsDevice() :
   QOpenGLExtraFunctions{ }
{
   initializeOpenGLFunctions();
}
