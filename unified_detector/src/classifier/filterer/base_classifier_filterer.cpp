#include "base_classifier_filterer.hpp"
#include "tag_filterer_utils.hpp"

BaseClassifierFilterer::BaseClassifierFilterer(std::shared_ptr<DataHandler> data_handler, bool allow_threshold)
  :IClassiferFilterer(data_handler){
    allow_thresholding = allow_threshold;
}
BaseClassifierFilterer::~BaseClassifierFilterer(){

}
std::vector<Entity> BaseClassifierFilterer::filter(std::vector<Entity> entities){
    for(auto it = entities.begin(); it != entities.end();){
        std::string display_tag = it->display_tag;        
        if (std::find(strict_fg_categories.begin(), strict_fg_categories.end(), display_tag) != strict_fg_categories.end()) {
            it = entities.erase(it);
        }
        else{
            it++;
        }
    }
#ifndef TESTING
    if(allow_thresholding) //enabled in 1.5.7
    entities = TagFiltererUtils::apply_thresholding(entities, data_handler);
#endif
    entities = TagFiltererUtils::assign_cluster_tags(entities, data_handler);
    return entities;
}