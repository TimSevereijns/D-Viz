#ifndef SQUARIFIEDTREEMAP_H
#define SQUARIFIEDTREEMAP_H

#include "visualization.h"

class SquarifiedTreeMap : public Visualization
{
   public:
      SquarifiedTreeMap(const std::wstring& rawPath);
      ~SquarifiedTreeMap();

      void ParseScan() override;
};

#endif // SQUARIFIEDTREEMAP_H
