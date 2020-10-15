#ifndef VIEWFACTORYINTERFACE_H
#define VIEWFACTORYINTERFACE_H

#include <memory>

#include "View/baseView.h"

class Controller;

class ViewFactoryInterface
{
  public:
    virtual std::shared_ptr<BaseView> CreateView(Controller& /*controller*/) const
    {
        return nullptr;
    }
};

#endif // VIEWFACTORYINTERFACE_H
