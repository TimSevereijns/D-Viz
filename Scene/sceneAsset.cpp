#include "sceneAsset.h"

#include <iostream>
#include <utility>

SceneAsset::SceneAsset(GraphicsDevice& device)
   : m_graphicsDevice(device)
{
}

SceneAsset::~SceneAsset()
{
   ClearBuffers();
}

bool SceneAsset::ClearBuffers()
{
   m_vertexBuffer.destroy();
   m_colorBuffer.destroy();

   return true;
}

bool SceneAsset::LoadShaders(const QString& vertexShaderName, const QString& fragmentShaderName)
{
   if (!m_shader.addShaderFromSourceFile(QOpenGLShader::Vertex,
      ":/Shaders/" + vertexShaderName + ".vert"))
   {
      std::cout << "Error loading vertex shader!" << std::endl;
   }

   if (!m_shader.addShaderFromSourceFile(QOpenGLShader::Fragment,
      ":/Shaders/" + fragmentShaderName + ".frag"))
   {
      std::cout << "Error loading fragment shader!" << std::endl;
   }

   return m_shader.link();
}

QOpenGLShaderProgram& SceneAsset::GetVertexShader()
{
   return m_shader;
}

bool SceneAsset::IsAssetLoaded() const
{
   return !(m_rawVertices.empty() && m_rawColors.empty());
}

void SceneAsset::SetVertexData(QVector<QVector3D>&& data)
{
   m_rawVertices.append(std::forward<QVector<QVector3D>>(data));
}

void SceneAsset::SetColorData(QVector<QVector3D>&& data)
{
   m_rawColors.append(std::forward<QVector<QVector3D>>(data));
}

unsigned int SceneAsset::GetVertexCount() const
{
   return static_cast<unsigned int>(m_rawVertices.size());
}

unsigned int SceneAsset::GetColorCount() const
{
   return static_cast<unsigned int>(m_rawColors.size());
}
