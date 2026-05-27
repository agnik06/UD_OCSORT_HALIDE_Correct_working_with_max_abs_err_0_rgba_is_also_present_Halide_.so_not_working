#include "tag_mapper.hpp"
#include <queue>
#include <algorithm>
#include "logger/logger.hpp"


ITagMapper::ITagMapper(std::shared_ptr<DataHandler> data_handler)
    :data_handler(data_handler){

}
ITagMapper::~ITagMapper(){

}

std::vector<float> ITagMapper::suppress_baselines(std::vector<float> probs){
    data_handler->suppressBaseProbs(probs);
    return probs;
}

std::vector<Entity> ITagMapper::get_display_entities(const std::vector<float>& probs){
    if(probs.empty()){
        LOGV("VZ Debug: expected probs.size() > 0");
        return {};
    }
    if(probs.size() != data_handler->getBaseCount()){
        LOGV("VZ Debug: expected probs.size() = no. of base tags");
        return {};
    }

    std::vector<Entity> display_entities;
    //Compute display tag scores
    for (int display_id = 0; display_id < data_handler->getDisplayCount(); display_id++) {
        std::string display_tag = data_handler->getDisplayTag(display_id);
        Entity entity(display_tag);

        //Populate base probs of display_tag
        std::vector<int> base_ids = data_handler->getBaseIdsFromDisplay(display_tag);
        std::vector<float> base_probs;
        std::vector<std::string> base_tags;
        
        base_probs.reserve(base_ids.size() + 1);
        base_tags.reserve (base_ids.size() + 1);

        for (int base_id : base_ids ) {
            base_probs.push_back(probs[base_id]);
            try{
                base_tags.push_back(data_handler->getBaseTag(base_id));
            }catch(const std::exception& e){
                std::cerr << e.what() << '\n';
                LOGV("VZ Debug: %s", e.what());
            }catch(...){
                LOGV("VZ Debug: Unknown exception occured in get_display_entities");
            }
            
        }

        std::vector<int> base_indices_top;
        std::vector<float> base_probs_top;
        get_top_n(base_indices_top, base_probs_top, base_probs, std::min(3, (int)base_probs.size()));

        float display_prob_value = 0.0;
        for (int k = 0; k < base_probs_top.size(); k++) {
            display_prob_value += base_probs_top[k];
            if(display_tag == "Others") break; //Only top-1 is considered for Others else top1-3 are considered
        }

        entity.display_tag_score = display_prob_value;
        if (!base_probs_top.empty()) {
            entity.baseline_tag = base_tags[base_indices_top[0]];
            entity.baseline_tag_score = base_probs_top[0];
        }
        display_entities.push_back(entity);
    }

    //Sort display entities & rearrange backup categories
    std::sort(display_entities.begin(), display_entities.end(), 
        [&](const Entity& lhs, const Entity& rhs) {
        int lhs_priority = data_handler->getDisplayTagPriority(lhs.display_tag);
        int rhs_priority = data_handler->getDisplayTagPriority(rhs.display_tag);
        if(lhs_priority == rhs_priority) return lhs.display_tag_score > rhs.display_tag_score;
        return lhs_priority < rhs_priority;
        }
    );

    //Loading thresholds
    for(Entity& entity: display_entities){
        entity.threshold = data_handler->getDisplayThreshold(entity.display_tag);
    }
    return display_entities;
}

std::vector<Entity> ITagMapper::sort_display_entities(std::vector<Entity> entities){
    //order according to: 1)Entities crossing threshold  2)Backup category to last 3)score comparison
    std::sort(entities.begin(), entities.end(), [&](const Entity &lhs, const Entity &rhs){
        bool lhs_check = lhs.display_tag_score >= data_handler->getDisplayThreshold(lhs.display_tag);
        bool rhs_check = rhs.display_tag_score >= data_handler->getDisplayThreshold(rhs.display_tag);
        if(lhs_check ^ rhs_check) return lhs_check;//if only one of them has crossed threshold
        // if(lhs_check & rhs_check){ //both crossed threshold
            int lhs_priority = data_handler->getDisplayTagPriority(lhs.display_tag);
            int rhs_priority = data_handler->getDisplayTagPriority(rhs.display_tag);
            if(lhs_priority != rhs_priority) return lhs_priority < rhs_priority;
        // }
        return lhs.display_tag_score > rhs.display_tag_score;
    });
    //Map to the cluster
    // for(Entity& entity: entities){
    //     entity.display_tag = data_handler->getDisplayClusterTag(entity.display_tag);
    // }
    // //remove duplicate entities
    // std::vector<std::string> display_tags;
    // for(auto it = entities.begin(); it!=entities.end(); ){
    //     if(std::find(display_tags.begin(), display_tags.end(), (*it).display_tag) != display_tags.end() ){
    //         it = entities.erase(it);
    //     }
    //     else{
    //         display_tags.push_back((*it).display_tag);
    //         it++;
    //     }
    // }
    return entities;
}

std::vector<Entity> ITagMapper::normalize_scores(std::vector<Entity> entities){
    std::for_each(entities.begin(), entities.end(), [&](Entity &entity){
        if (entity.display_tag_score <= normalized_threshold)
            entity.display_tag_score = (0.5 * (entity.display_tag_score / normalized_threshold));
        else
            entity.display_tag_score =  (0.5 + 0.5 * (entity.display_tag_score) / (1.0 - normalized_threshold));
    });
    return entities;
}

void ITagMapper::get_top_n(std::vector<int>& top_indices, std::vector<float>& top_probs, std::vector<float>& scores,const int beam_size) {
    top_indices.clear();
    top_probs.clear();

    top_indices.resize(beam_size);
    top_probs.resize(beam_size);

    auto compare_fn = [](std::pair<int, float>& a, std::pair<int, float>& b) {
        return a.second < b.second;
    };
    
    std::priority_queue<std::pair<int, float>, std::vector<std::pair<int, float> >, decltype(compare_fn)> best_indices(compare_fn);
    for (int i = 0; i < scores.size(); i++) {
        std::pair<int, float> index_score = std::make_pair(i, scores[i]);
        best_indices.push(index_score);
    }

    for (int i = 0; i < beam_size; i++) {
        top_indices[i] = best_indices.top().first;
        top_probs[i] = best_indices.top().second;
        best_indices.pop();
    }
}