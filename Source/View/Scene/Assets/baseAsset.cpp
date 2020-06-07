#include "View/Scene/Assets/baseAsset.h"

#include "constants.h"
#include "controller.h"

#include <spdlog/spdlog.h>

#include <utility>

namespace Assets
{
    AssetBase::AssetBase(const Controller& controller, QOpenGLExtraFunctions& openGL)
        : m_openGL{ openGL },
          m_persistentSettings{ controller.GetPersistentSettings() },
          m_sessionSettings{ controller.GetSessionSettings() }
    {
    }

    void AssetBase::ClearBuffers()
    {
        m_rawVertices.clear();
        m_rawColors.clear();
    }

    bool AssetBase::LoadShaders(const QString& vertexShaderName, const QString& fragmentShaderName)
    {
        if (!m_mainShader.addShaderFromSourceFile(
                QOpenGLShader::Vertex, ":/View/Shaders/" + vertexShaderName + ".vert")) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Failed to load vertex shader: " + vertexShaderName.toStdString() + ".vert");

            return false;
        }

        if (!m_mainShader.addShaderFromSourceFile(
                QOpenGLShader::Fragment, ":/View/Shaders/" + fragmentShaderName + ".frag")) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error(
                "Failed to load fragment shader: " + fragmentShaderName.toStdString() + ".frag");

            return false;
        }

        const auto linkedSuccessfully = m_mainShader.link();
        if (!linkedSuccessfully) {
            const auto& log = spdlog::get(Constants::Logging::DefaultLog);
            log->error("Failed to link the shader program!");

            return false;
        }

        return true;
    }

    bool AssetBase::IsAssetLoaded() const
    {
        return !(m_rawVertices.empty() && m_rawColors.empty());
    }

    void AssetBase::SetVertexCoordinates(QVector<QVector3D>&& data)
    {
        m_rawVertices.clear();
        m_rawVertices.append(data);
    }

    void AssetBase::SetVertexColors(QVector<QVector3D>&& data)
    {
        m_rawColors.clear();
        m_rawColors.append(data);
    }

    void AssetBase::AddVertexCoordinates(QVector<QVector3D>&& positionData)
    {
        m_rawVertices.append(positionData);
    }

    void AssetBase::AddVertexColors(QVector<QVector3D>&& colorData)
    {
        m_rawColors.append(colorData);
    }

    unsigned int AssetBase::GetVertexCount() const
    {
        return static_cast<unsigned int>(m_rawVertices.size());
    }

    unsigned int AssetBase::GetColorCount() const
    {
        return static_cast<unsigned int>(m_rawColors.size());
    }

    void AssetBase::Show()
    {
        m_shouldRender = true;
    }

    void AssetBase::Hide()
    {
        m_shouldRender = false;
    }
} // namespace Assets
