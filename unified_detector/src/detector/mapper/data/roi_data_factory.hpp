#ifndef __ROI_DATA_FACTORY__
#define __ROI_DATA_FACTORY__

#include "core/model_config.hpp"
#include "json.hpp"

using json = nlohmann::json;

json get_det_json_str_v1();

class ROIDataFactory{
public:
    ROIDataFactory();
    static json get_data(model::Version version);
};
#endif