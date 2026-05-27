#ifndef __TAG_FILTERER_UTILS__
#define __TAG_FILTERER_UTILS__

#include <vector>
#include <memory>
#include "framework/entity.hpp"
#include "../data_handler/data_handler.hpp"

//Static Class
class TagFiltererUtils{
public:
    static std::vector<Entity> assign_cluster_tags(std::vector<Entity> entities, std::shared_ptr<DataHandler> data_handler);
    static std::vector<Entity> apply_thresholding(std::vector<Entity> entities, std::shared_ptr<DataHandler> data_handler);    
    static std::vector<Entity> apply_thresholding_from_entity(std::vector<Entity> entities);
};



#endif //__TAG_FILTERER_UTILS__