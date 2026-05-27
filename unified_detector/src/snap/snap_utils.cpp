#include "snap_utils.hpp"

void snap_utils::get_output_dimensions( std::vector<snap::DataBuffer> outputs, int &b,int &c,int &h,int &w,std::string model){
    if(outputs[0].dataFormat == snap::DataFormat::NCHW) {
        b = outputs[0].shape[0];
        c = outputs[0].shape[1];
        h = outputs[0].shape[2];
        w = outputs[0].shape[3];
        LOGV("SNAP output shape for [%s] is [%s] and dimensions are N :[%d], C :[%d], H :[%d], W :[%d]", model.c_str(), outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW",outputs[0].shape[0],outputs[0].shape[1],outputs[0].shape[2],outputs[0].shape[3]);
      } else {
        b = outputs[0].shape[0];
        h = outputs[0].shape[1];
        w = outputs[0].shape[2];
        c = outputs[0].shape[3];
        LOGV("SNAP output shape for [%s] is [%s] and dimensions are N :[%d], H :[%d], W :[%d], C :[%d]", model.c_str(), outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW",outputs[0].shape[0],outputs[0].shape[1],outputs[0].shape[2],outputs[0].shape[3]);
      }
}

void snap_utils::prepare_snap_input(cv::Mat input_image, std::vector<snap::DataBuffer> &inputs, int input_h, int input_w, int input_c, int comp_unit,std::string model){
    struct snap::DataBuffer snap_input;
    snap_input.data = input_image.data;
    snap_input.dataType = snap::DataBufferType::FLOAT32;
    snap_input.shape = {1, input_h, input_w, input_c};
    snap_input.dataFormat = snap::DataFormat::NHWC;

    /*if(model == "detector" || comp_unit != 3){ //  NPU Classifier input shape is NHWC for all except NPU classifier which is NCHW.
        snap_input.shape = {1, input_h, input_w, input_c};
        snap_input.dataFormat = snap::DataFormat::NHWC;
        LOGV("SNAP input shape for [%s] is [%s] and dimensions are N :[%d], H :[%d], W :[%d], C :[%d]", model.c_str(), snap_input.dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW",snap_input.shape[0],snap_input.shape[1],snap_input.shape[2],snap_input.shape[3]);
    }else{
        snap_input.shape = {1, input_c,input_h, input_w};
        snap_input.dataFormat = snap::DataFormat::NCHW;
        LOGV("SNAP input shape for [%s] is [%s] and dimensions are N :[%d], C :[%d], H :[%d], W :[%d]", model.c_str(), snap_input.dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW",snap_input.shape[0],snap_input.shape[1],snap_input.shape[2],snap_input.shape[3]);
    }*/
    inputs.push_back(snap_input);
}