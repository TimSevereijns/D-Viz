#ifndef TEXTUREPREVIEWASSET_H
#define TEXTUREPREVIEWASSET_H

#include <memory>

#include <QImage>
#include <QOpenGLTexture>

#include "baseAsset.h"

namespace Asset
{
   class TexturePreview : public Base
   {
   public:

      explicit TexturePreview(
         QOpenGLExtraFunctions& openGL,
         bool isInitiallyVisible);

      bool Initialize() override;

      bool LoadShaders() override;

      bool Render(
         const Camera &camera,
         const std::vector<Light>& light,
         const Settings::Manager& settings) override;

      bool Refresh() override;

      void SetTexture(const QImage &texture);

   private:

      QOpenGLBuffer m_VBO;

      std::unique_ptr<QOpenGLTexture> m_texture;
   };
}

#endif // TEXTUREPREVIEWASSET_H
