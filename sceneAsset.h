#ifndef SCENEASSET_H
#define SCENEASSET_H

#include <QOpenGLShaderProgram>

/**
 * @brief The SceneAsset class
 */
class SceneAsset
{
   public:
      explicit SceneAsset();
      ~SceneAsset();

      bool PrepareVertexBuffers();
      bool PrepareColorBuffers();

      bool ClearBuffers();

      bool LoadShaders(const QString& shaderLocation);
      QOpenGLShaderProgram& GetVertexShader();
      QOpenGLShaderProgram& GetFragmentShader();

      bool Render();

   private:
      QOpenGLShaderProgram m_vertexShader;
      QOpenGLShaderProgram m_fragmentShader;
};

#endif // SCENEASSET_H
