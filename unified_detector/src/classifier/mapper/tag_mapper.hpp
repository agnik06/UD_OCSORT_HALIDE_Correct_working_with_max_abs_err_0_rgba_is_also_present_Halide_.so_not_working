#ifndef __TAG_MAPPER_INTERFACE__
#define __TAG_MAPPER_INTERFACE__

#include <vector>
#include "framework/entity.hpp"
#include "../data_handler/data_handler.hpp"

class ITagMapper{
public:
    ITagMapper(std::shared_ptr<DataHandler> data_handler);
    ~ITagMapper();
    virtual std::vector<Entity> map(std::vector<float> probs) = 0;

protected:
    std::shared_ptr<DataHandler> data_handler;
    float normalized_threshold = 0.5f;
    
    std::vector<float> suppress_baselines(std::vector<float> probs);
    std::vector<Entity> get_display_entities(const std::vector<float>& probs);
    std::vector<Entity> sort_display_entities(std::vector<Entity> entities);
    std::vector<Entity> normalize_scores(std::vector<Entity> entities);
    void get_top_n(std::vector<int>& indices, std::vector<float>& probs, std::vector<float>& y,const int beam_size);
    
};

#endif //__TAG_MAPPER_INTERFACE__