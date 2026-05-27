#ifndef __FASE_CLASSIFIER_FILTERER__
#define __FASE_CLASSIFIER_FILTERER__

#include "classifier_filterer.hpp"
class BaseClassifierFilterer: public IClassiferFilterer{
public:
    BaseClassifierFilterer(std::shared_ptr<DataHandler> data_handler, bool allow_threshold=true);
    ~BaseClassifierFilterer();
    std::vector<Entity> filter(std::vector<Entity> entities);

private:
    std::vector<std::string> strict_fg_categories = {"Animal", "Bird", "Vehicle"};
    bool allow_thresholding;
};

#endif //__FASE_CLASSIFIER_FILTERER__