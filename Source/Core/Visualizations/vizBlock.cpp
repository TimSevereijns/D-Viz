#include "Visualizations/vizBlock.h"

VizBlock::VizBlock(FileInfo file) : file{ std::move(file) }
{
}

VizBlock::VizBlock(FileInfo file, const Block& block)
    : file{ std::move(file) }, block{ block }, boundingBox{ block }
{
}
