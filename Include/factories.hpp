#ifndef FACTORIES_HPP
#define FACTORIES_HPP

#include <filesystem>
#include <memory>
#include <string>

#include "Monitor/fileMonitorBase.h"
#include "Visualizations/baseModel.h"
#include "Visualizations/squarifiedTreemap.h"
#include "Windows/baseView.h"
#include "Windows/mainWindow.h"

class Controller;

class ViewFactoryInterface
{
  public:
    virtual auto CreateView(Controller& /*controller*/) const -> std::shared_ptr<BaseView>
    {
        return nullptr;
    }
};

class ViewFactory final : public ViewFactoryInterface
{
  public:
    auto CreateView(Controller& controller) const -> std::shared_ptr<BaseView> override
    {
        return std::make_shared<MainWindow>(controller);
    }
};

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

#endif // FACTORIES_HPP
