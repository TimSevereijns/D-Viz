#include "sceneAsset.h"

SceneAsset::SceneAsset()
{
}

SceneAsset::~SceneAsset()
{
}

bool SceneAsset::PrepareVertexBuffers()
{
   return false;
}

bool SceneAsset::PrepareColorBuffers()
{
   return false;
}

bool SceneAsset::ClearBuffers()
{
   return false;
}

bool SceneAsset::LoadShaders(const QString& shaderLocation)
{
   return false;
}

QOpenGLShaderProgram& SceneAsset::GetVertexShader()
{
   return m_vertexShader;
}

QOpenGLShaderProgram& SceneAsset::GetFragmentShader()
{
   return m_fragmentShader;
}

bool SceneAsset::Render()
{
   return false;
}
