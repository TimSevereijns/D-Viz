#ifndef SQUARIFIEDTREEMAP_H
#define SQUARIFIEDTREEMAP_H

#include "visualization.h"

#include <cstdint>

/**
 * @brief The RealEstate struct
 */
struct RealEstate
{
   Block m_block;
   std::uintmax_t m_fileSize;

   RealEstate() {};

   RealEstate(const Block& block, const std::uintmax_t size)
      : m_block(block),
        m_fileSize(size)
   {
   }
};

class SquarifiedTreeMap : public Visualization
{
   public:
      SquarifiedTreeMap(const std::wstring& rawPath);
      ~SquarifiedTreeMap();

      void ParseScan() override;
};

#endif // SQUARIFIEDTREEMAP_H
