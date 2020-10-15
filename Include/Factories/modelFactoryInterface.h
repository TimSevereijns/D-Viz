#ifndef MODELFACTORYINTERFACE_H
#define MODELFACTORYINTERFACE_H

#include <filesystem>
#include <memory>

#include "Model/Monitor/fileMonitorBase.h"
#include "Model/baseModel.h"

class ModelFactoryInterface
{
  public:
    virtual std::shared_ptr<BaseModel> CreateModel(
        std::unique_ptr<FileMonitorBase> /*fileMonitor*/,
        const std::filesystem::path& /*path*/) const
    {
        return nullptr;
    };
};

#endif // MODELFACTORYINTERFACE_H
