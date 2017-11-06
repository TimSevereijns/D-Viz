#ifndef LINEASSET_H
#define LINEASSET_H

#include "baseAsset.h"

namespace Asset
{
   /**
    * @brief The LineAsset class
    */
   class Line : public Base
   {
      public:

         explicit Line(QOpenGLExtraFunctions& openGL);

         /**
          * @see Asset::Base::LoadShaders(...)
          */
         bool LoadShaders() override;

         /**
          * @see Asset::Base::Initialize(...)
          */
         bool Initialize() override;

         /**
          * @see Asset::Base::Render(...)
          */
         bool Render(
            const Camera& camera,
            const std::vector<Light>& light,
            const OptionsManager& settings) override;

         /**
          * @see Asset::Base::Reload(...)
          */
         bool Reload() override;

      private:

         bool InitializeVertexBuffers();

         bool InitializeColorBuffers();
   };
}

#endif // LINEASSET_H
