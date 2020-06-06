#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include "Factories/modelFactoryInterface.h"
#include "Model/squarifiedTreemap.h"

class ModelFactory final : public ModelFactoryInterface
{
  public:
    auto CreateModel(
        std::unique_ptr<FileMonitorBase> fileMonitor, const std::filesystem::path& path) const
        -> std::shared_ptr<BaseModel> override
    {
        return std::make_shared<SquarifiedTreeMap>(std::move(fileMonitor), path);
    }
};

#endif // MODELFACTORY_H
