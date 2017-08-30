#include "sceneAsset.h"

#include <iostream>
#include <utility>

SceneAsset::SceneAsset(QOpenGLExtraFunctions& device) :
   m_graphicsDevice{ device }
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

bool SceneAsset::LoadShaders(
   const QString& vertexShaderName,
   const QString& fragmentShaderName)
{
   if (!m_mainShader.addShaderFromSourceFile(QOpenGLShader::Vertex,
      ":/Shaders/" + vertexShaderName + ".vert"))
   {
      std::cout << "Error loading vertex shader!" << std::endl;
   }

   if (!m_mainShader.addShaderFromSourceFile(QOpenGLShader::Fragment,
      ":/Shaders/" + fragmentShaderName + ".frag"))
   {
      std::cout << "Error loading fragment shader!" << std::endl;
   }

   return m_mainShader.link();
}

bool SceneAsset::IsAssetLoaded() const
{
   return !(m_rawVertices.empty() && m_rawColors.empty());
}

void SceneAsset::SetVertexCoordinates(QVector<QVector3D>&& data)
{
   m_rawVertices.clear();
   m_rawVertices.append(std::move(data));
}

void SceneAsset::SetVertexColors(QVector<QVector3D>&& data)
{
   m_rawColors.clear();
   m_rawColors.append(std::move(data));
}

void SceneAsset::AddVertexCoordinates(QVector<QVector3D>&& positionData)
{
   m_rawVertices.append(std::move(positionData));
}

void SceneAsset::AddVertexColors(QVector<QVector3D>&& colorData)
{
   m_rawColors.append(std::move(colorData));
}

unsigned int SceneAsset::GetVertexCount() const
{
   return static_cast<unsigned int>(m_rawVertices.size());
}

unsigned int SceneAsset::GetColorCount() const
{
   return static_cast<unsigned int>(m_rawColors.size());
}

void SceneAsset::UpdateVBO(
   const Tree<VizFile>::Node&,
   UpdateAction,
   const VisualizationParameters&)
{
}
