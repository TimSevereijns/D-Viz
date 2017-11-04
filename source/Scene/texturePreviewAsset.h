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

      explicit TexturePreview(QOpenGLExtraFunctions& openGL);

      bool Initialize() override;

      bool LoadShaders() override;

      bool Render(
         const Camera &camera,
         const std::vector<Light>& light,
         const OptionsManager& settings) override;

      bool Reload() override;

      void SetTexture(const QImage &texture);

   private:

      QOpenGLBuffer m_VBO;

      std::unique_ptr<QOpenGLTexture> m_texture;
   };
}

#endif // TEXTUREPREVIEWASSET_H
