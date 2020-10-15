#ifndef MODELFACTORY_H
#define MODELFACTORY_H

#include "Factories/modelFactoryInterface.h"
#include "Model/squarifiedTreemap.h"

class ModelFactory final : public ModelFactoryInterface
{
  public:
    std::shared_ptr<BaseModel> CreateModel(
        std::unique_ptr<FileMonitorBase> fileMonitor,
        const std::filesystem::path& path) const override
    {
        return std::make_shared<SquarifiedTreeMap>(std::move(fileMonitor), path);
    }
};

#endif // MODELFACTORY_H
