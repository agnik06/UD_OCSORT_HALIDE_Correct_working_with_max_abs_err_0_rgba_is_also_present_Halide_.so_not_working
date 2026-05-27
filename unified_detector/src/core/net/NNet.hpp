#ifndef NNET_HPP
#define NNET_HPP
#include <vector>
#include <opencv2/opencv.hpp>
#include <string>

#include "DLInterface_C_Wrapper.hpp"
#include "logger/logger.hpp"
#include "model_interface/dl_loader.hpp"
#include <dlfcn.h>
#include <thread>
#include <mutex>
#include <functional>
 #include "sys/stat.h"

typedef std::function<void(bool, std::string)> NNetInitCallback;

class NNet{
    public:
        bool init_net(const std::string& file_path, const std::string& moduleName, std::vector<std::string> layer_names, std::vector<float> channel_means, int compute_unit_preference, int execution_data_type_preference,NNetInitCallback cb);
        NNet(const std::string& file_path, const std::string& moduleName, std::vector<std::string> layer_names, std::vector<float> channel_means, int compute_unit_preference, int execution_data_type_preference, NNetInitCallback cb,bool &status);
        bool forward_pass(cv::Mat image, std::string& clasifer_or_detector);
		float* get_layer_output(const char* layerName, int &b, int &c, int &h, int &w);
        float* get_layer_output_with_format(const char* layerName, int &b, int &c, int &h, int &w, int &data_format);
        float* get_layer_output_shape(const char* layerName, std::vector<int>& output_shape);
        // bool get_open_status();
	    void get_target_dimen(int &t_height,int& t_width);
        void set_snap_options(const std::string& file_path, 
                                const std::string& module_name,
                                std::vector<std::string> layer_names, 
                                std::vector<float> channel_means, 
                                int compute_unit_preference, 
                                int execution_data_type_preference);
        std::string classifier_op_name = "";
        ~NNet();

    protected:
        DLIWrapper* dli_handle;
        std::string module_name;
        void* dlinterface_lib_handle;
        SNAPInterface snap_options;
        int target_width, target_height, target_channels;
        bool open_status = false;
        bool execute_status;
        int input_image_cnt=0;
        
        std::thread init_thread;
        std::mutex init_mutex;
        std::mutex deinit_mutex;
        int compute_unit;
        int original_image_cnt =0;
        std::vector<float> channel_means, channel_scales;
        #ifndef SQA_ANALYSIS
            bool set_channel_means(std::vector<float> means);
        #endif
};

#endif /* NNET_HPP */
