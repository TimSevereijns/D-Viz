#ifndef MODELFACTORYINTERFACE_H
#define MODELFACTORYINTERFACE_H

#include <filesystem>
#include <memory>

#include "Monitor/fileMonitorBase.h"
#include "Visualizations/baseModel.h"

class ModelFactoryInterface
{
  public:
    virtual auto CreateModel(
        std::unique_ptr<FileMonitorBase> /*fileMonitor*/,
        const std::filesystem::path& /*path*/) const -> std::shared_ptr<BaseModel>
    {
        return nullptr;
    };
};

#endif // MODELFACTORYINTERFACE_H
