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

         bool LoadShaders() override;

         bool Initialize() override;

         bool Render(
            const Camera& camera,
            const std::vector<Light>& light,
            const OptionsManager& settings) override;

         bool Reload() override;

      private:

         bool InitializeVertexBuffers();

         bool InitializeColorBuffers();
   };
}

#endif // LINEASSET_H
