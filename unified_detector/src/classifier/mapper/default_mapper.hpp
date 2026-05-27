#ifndef __DEFAULT_MAPPER__
#define __DEFAULT_MAPPER__

#include "tag_mapper.hpp"

class DefaultMapper: public ITagMapper{
public:
    DefaultMapper(std::shared_ptr<DataHandler> data_handler);
    ~DefaultMapper();
    std::vector<Entity> map(std::vector<float> probs);
};

#endif //__DEFAULT_MAPPER__