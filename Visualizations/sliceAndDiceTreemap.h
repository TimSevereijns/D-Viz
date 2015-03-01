#ifndef SLICEANDDICETREEMAP_H
#define SLICEANDDICETREEMAP_H

#include "visualization.h"

class SliceAndDiceTreeMap : public Visualization
{
   public:
      SliceAndDiceTreeMap(const std::wstring& rawPath);
      ~SliceAndDiceTreeMap();

      void ParseScan() override;
};

#endif // SLICEANDDICETREEMAP_H
