#include "baseAsset.h"

#include "constants.h"

#include <spdlog/spdlog.h>

#include <iostream>
#include <utility>

namespace Asset
{
   Base::Base(const Settings::Manager& settings,
      QOpenGLExtraFunctions& openGL)
      :
      m_openGL{ openGL },
      m_settingsManager{ settings }
   {
   }

   void Base::ClearBuffers()
   {
      m_rawVertices.clear();
      m_rawColors.clear();
   }

   bool Base::LoadShaders(
      const QString& vertexShaderName,
      const QString& fragmentShaderName)
   {
      if (!m_mainShader.addShaderFromSourceFile(QOpenGLShader::Vertex,
         ":/Shaders/" + vertexShaderName + ".vert"))
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Failed to load vertex shader: " + vertexShaderName.toStdString() + ".vert");

         return false;
      }

      if (!m_mainShader.addShaderFromSourceFile(QOpenGLShader::Fragment,
         ":/Shaders/" + fragmentShaderName + ".frag"))
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Failed to load fragment shader: " + fragmentShaderName.toStdString() + ".frag");

         return false;
      }

      const auto linkedSuccessfully = m_mainShader.link();
      if (!linkedSuccessfully)
      {
         const auto& log = spdlog::get(Constants::Logging::DEFAULT_LOG);
         log->error("Failed to link the shader program!");

         return false;
      }

      return true;
   }

   bool Base::DetermineVisibilityFromPreferences(std::wstring_view assetName)
   {
      const auto preferenceName = std::wstring{ L"show" } + assetName.data();
      const auto& preferences = m_settingsManager.GetPreferenceMap();
      const auto shouldRender = preferences.GetValueOrDefault(preferenceName, true);

      return shouldRender;
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
      Asset::Event)
   {
   }
}
