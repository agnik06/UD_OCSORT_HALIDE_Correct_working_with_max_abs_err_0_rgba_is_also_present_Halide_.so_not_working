#ifndef __DATA_FACTORY__
#define __DATA_FACTORY__


#include "json.hpp"
#include "core/model_config.hpp"

using json = nlohmann::json;


json get_json_str_v1();

class DataFactory{
public:
    DataFactory();
    static json get_data(model::Version version);
};
#endif