#ifndef DEBUGGINGRAYASSET_H
#define DEBUGGINGRAYASSET_H

#include "lineAsset.h"

namespace Assets
{
    class DebuggingRay final : public Line
    {
      public:
        /**
         * @copydoc Asset::Base::Base()
         */
        DebuggingRay(const Controller& controller, QOpenGLExtraFunctions& openGL);

        /**
         * @copydoc Asset::Base::Render()
         */
        void Render(const Camera& camera, const std::vector<Light>& lights) override;
    };
} // namespace Assets

#endif // DEBUGGINGRAYASSET_H
