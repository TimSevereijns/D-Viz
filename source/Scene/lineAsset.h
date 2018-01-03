#ifndef LINEASSET_H
#define LINEASSET_H

#include "baseAsset.h"

namespace Asset
{
   /**
    * @brief The Line Asset class can be used to render any scene asset that is composed primarily
    * of lines.
    */
   class Line : public Base
   {
      public:

         /**
          * @see Asset::Base::Base(...)
          */
         Line(
            QOpenGLExtraFunctions& openGL,
            bool isInitiallyVisible);

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
            const Settings::Manager& settings) override;

         /**
          * @see Asset::Base::Reload(...)
          */
         bool Refresh() override;

      private:

         bool InitializeVertexBuffers();

         bool InitializeColorBuffers();
   };
}

#endif // LINEASSET_H
