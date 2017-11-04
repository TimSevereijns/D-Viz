#include "baseAsset.h"

#include <iostream>
#include <utility>

namespace Asset
{
   Base::Base(QOpenGLExtraFunctions& openGL) :
      m_openGL{ openGL }
   {
   }

   bool Base::ClearBuffers()
   {
      m_rawVertices.clear();
      m_rawColors.clear();

      return true;
   }

   bool Base::LoadShaders(
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

   bool Base::IsAssetLoaded() const
   {
      return !(m_rawVertices.empty() && m_rawColors.empty());
   }

   void Base::SetVertexCoordinates(QVector<QVector3D>&& data)
   {
      m_rawVertices.clear();
      m_rawVertices.append(std::move(data));
   }

   void Base::SetVertexColors(QVector<QVector3D>&& data)
   {
      m_rawColors.clear();
      m_rawColors.append(std::move(data));
   }

   void Base::AddVertexCoordinates(QVector<QVector3D>&& positionData)
   {
      m_rawVertices.append(std::move(positionData));
   }

   void Base::AddVertexColors(QVector<QVector3D>&& colorData)
   {
      m_rawColors.append(std::move(colorData));
   }

   unsigned int Base::GetVertexCount() const
   {
      return static_cast<unsigned int>(m_rawVertices.size());
   }

   unsigned int Base::GetColorCount() const
   {
      return static_cast<unsigned int>(m_rawColors.size());
   }

   void Base::Show()
   {
      m_shouldRender = true;
   }

   void Base::Hide()
   {
      m_shouldRender = false;
   }

   void Base::UpdateVBO(
      const Tree<VizFile>::Node&,
      UpdateAction,
      const VisualizationParameters&)
   {
   }
}
