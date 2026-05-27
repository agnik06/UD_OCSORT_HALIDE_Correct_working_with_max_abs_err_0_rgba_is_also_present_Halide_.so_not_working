#include "default_mapper.hpp"

DefaultMapper::DefaultMapper(std::shared_ptr<DataHandler> data_handler)
    :ITagMapper(data_handler){

}

DefaultMapper::~DefaultMapper(){

}

std::vector<Entity> DefaultMapper::map(std::vector<float> probs){
    probs = suppress_baselines(probs);
    std::vector<Entity> display_entities = get_display_entities(probs);
    display_entities = sort_display_entities(display_entities);
    // display_entities = normalize_scores(display_entities);
    return display_entities;
}