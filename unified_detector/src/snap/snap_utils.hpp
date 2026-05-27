/**
 * @file   snap_utils.hpp
 * @brief  This file implements the snap utils Interface
 *
 * This file implements snap utils to be used by end api's
 */
 #ifndef __SNAP_UTILS__
 #define __SNAP_UTILS__
 #include "snap_interface.h"
 #include <opencv2/core/core.hpp>
 #include "core/logger/logger.hpp"
 namespace snap_utils{
    void get_output_dimensions( std::vector<snap::DataBuffer> outputs,int &b,int &c,int &h,int &w,std::string model);
    void prepare_snap_input(cv::Mat input_image, std::vector<snap::DataBuffer> &inputs, int input_h, int input_w, int input_c, int comp_unit, std::string model);
  };

 #endif //__CLASSIFIER_UTILS__