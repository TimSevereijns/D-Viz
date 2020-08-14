#ifndef VIEWFACTORY_H
#define VIEWFACTORY_H

#include "Factories/viewFactoryInterface.h"
#include "View/mainWindow.h"
#include "controller.h"

class ViewFactory final : public ViewFactoryInterface
{
  public:
    auto CreateView(Controller& controller) const -> std::shared_ptr<BaseView> override
    {
        return std::make_shared<MainWindow>(controller);
    }
};

#endif // VIEWFACTORY_H
