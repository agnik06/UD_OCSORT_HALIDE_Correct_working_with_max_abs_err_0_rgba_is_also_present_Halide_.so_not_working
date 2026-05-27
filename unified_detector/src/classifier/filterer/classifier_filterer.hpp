#ifndef __CLASSIFIER_FILTERER_INTERFACE__
#define __CLASSIFIER_FILTERER_INTERFACE__

#include "../data_handler/data_handler.hpp"
#include "framework/entity.hpp"

class IClassiferFilterer{
public:
    IClassiferFilterer(std::shared_ptr<DataHandler> data_handler);
    virtual ~IClassiferFilterer(){};
    virtual std::vector<Entity> filter(std::vector<Entity> entities)=0;

protected:
    std::shared_ptr<DataHandler> data_handler;
};

#endif //__CLASSIFIER_FILTERER_INTERFACE__