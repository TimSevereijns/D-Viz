#ifndef GRAPHICSDEVICE_H
#define GRAPHICSDEVICE_H

#include <QOpenGLExtraFunctions>

/**
 * @brief The GraphicsDevice class represents the main OpenGL state machine.
 *
 * In other words, all OpenGL function calls for a given GLCanvas have to go through a single
 * "graphics device" object.
 */
class GraphicsDevice : public QOpenGLExtraFunctions
{
   public:
      /**
       * @brief Initializes the OpenGL state machine.
       */
      GraphicsDevice();
};

#endif // GRAPHICSDEVICE_H
