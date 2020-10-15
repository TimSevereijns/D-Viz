#ifndef VIEWFACTORY_H
#define VIEWFACTORY_H

#include "Factories/viewFactoryInterface.h"
#include "View/mainWindow.h"
#include "controller.h"

class ViewFactory final : public ViewFactoryInterface
{
  public:
    std::shared_ptr<BaseView> CreateView(Controller& controller) const override
    {
        return std::make_shared<MainWindow>(controller);
    }
};

#endif // VIEWFACTORY_H
