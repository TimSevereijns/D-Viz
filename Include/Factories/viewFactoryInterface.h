#ifndef VIEWFACTORYINTERFACE_H
#define VIEWFACTORYINTERFACE_H

#include <memory>

#include "View/baseView.h"

class Controller;

class ViewFactoryInterface
{
  public:
    virtual auto CreateView(Controller& /*controller*/) const -> std::shared_ptr<BaseView>
    {
        return nullptr;
    }
};

#endif // VIEWFACTORYINTERFACE_H
