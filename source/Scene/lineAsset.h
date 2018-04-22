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
          * @copydoc Asset::Base::Base()
          */
         Line(
            const Settings::Manager& settings,
            QOpenGLExtraFunctions& openGL);

         /**
          * @copydoc Asset::Base::LoadShaders()
          */
         bool LoadShaders() override;

         /**
          * @copydoc Asset::Base::Initialize()
          */
         void Initialize() override;

         /**
          * @copydoc Asset::Base::Render()
          */
         void Render(
            const Camera& camera,
            const std::vector<Light>& light) override;

         /**
          * @copydoc Asset::Base::Reload()
          */
         void Refresh() override;

      private:

         void InitializeVertexBuffers();

         void InitializeColorBuffers();
   };
}

#endif // LINEASSET_H
