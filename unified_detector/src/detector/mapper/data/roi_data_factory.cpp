#include "roi_data_factory.hpp"
#include "logger/logger.hpp"

ROIDataFactory::ROIDataFactory(){ }

json ROIDataFactory::get_data(model::Version version){
    switch(version){
        case model::Version::V1:
            LOGV("ROIDataFactory get_data() loading %d data", version);
            return get_det_json_str_v1();        
        default:
            LOGV("No data found for version: %d", version);
    }
    return NULL;
}
