#include "tag_filterer_utils.hpp"
#include "logger/logger.hpp"

std::vector<Entity> TagFiltererUtils::assign_cluster_tags(std::vector<Entity> entities, std::shared_ptr<DataHandler> data_handler){
    //Map to the cluster
    for(Entity& entity: entities){
        entity.display_tag = data_handler->getDisplayClusterTag(entity.display_tag);
    }
    //remove duplicate entities
    std::vector<std::string> display_tags;
    for(auto it = entities.begin(); it!=entities.end(); ){
        if(std::find(display_tags.begin(), display_tags.end(), (*it).display_tag) != display_tags.end() ){
            it = entities.erase(it);
        }
        else{
            display_tags.push_back((*it).display_tag);
            it++;
        }
    }
    return entities;
}

std::vector<Entity> TagFiltererUtils::apply_thresholding(std::vector<Entity> entities, std::shared_ptr<DataHandler> data_handler){
    for(auto it = entities.begin(); it != entities.end();){
        std::string display_tag = it->display_tag;
        float display_tag_score = it->display_tag_score;
        float display_tag_threshold = data_handler->getDisplayThreshold(display_tag);
        if(display_tag_score < display_tag_threshold){
            it = entities.erase(it);
        }
        else{
            it++;
        }
    }
    return entities;
}

std::vector<Entity> TagFiltererUtils::apply_thresholding_from_entity(std::vector<Entity> entities){
    for(auto it = entities.begin(); it != entities.end();){
        std::string display_tag = it->display_tag;
        float display_tag_score = it->display_tag_score;
        float display_tag_threshold = it->threshold;
        if(display_tag_score < display_tag_threshold){
            it = entities.erase(it);
        }
        else{
            it++;
        }
    }
    return entities;
}
