#include "data_factory.hpp"
 #include "core/logger/logger.hpp"

DataFactory::DataFactory(){ }

json DataFactory::get_data(model::Version version){
    switch(version){
        case model::Version::V1:
            return get_json_str_v1();
        default:
            LOGV("No data found for version: %d", version);
    }
    return NULL;
}
