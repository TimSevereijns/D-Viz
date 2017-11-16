#include "baseAsset.h"

#include "constants.h"

#include <spdlog/spdlog.h>

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
      if (!m_shader.addShaderFromSourceFile(QOpenGLShader::Vertex,
         ":/Shaders/" + vertexShaderName + ".vert"))
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Failed to load vertex shader: " + vertexShaderName.toStdString() + ".vert");

         return false;
      }

      if (!m_shader.addShaderFromSourceFile(QOpenGLShader::Fragment,
         ":/Shaders/" + fragmentShaderName + ".frag"))
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Failed to load fragment shader: " + fragmentShaderName.toStdString() + ".frag");

         return false;
      }

      const auto linkedSuccessfully = m_shader.link();
      if (!linkedSuccessfully)
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Failed to link the shader program!");

         return false;
      }

      return true;
   }

   bool Base::IsAssetLoaded() const
   {
      return !(m_rawVertices.empty() && m_rawColors.empty());
   }

   void Base::SetVertexData(QVector<QVector3D>&& data)
   {
      m_rawVertices.clear();
      m_rawVertices.append(std::move(data));
   }

   void Base::SetColorData(QVector<QVector3D>&& data)
   {
      m_rawColors.clear();
      m_rawColors.append(std::move(data));
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
      Asset::Event,
      const Settings::Manager&)
   {
   }
}
