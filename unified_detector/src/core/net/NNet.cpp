#include "NNet.hpp"
// #include "model_interface/image_preprocessor.hpp"
#include <stdexcept>
#include "utils/timer.hpp"
#include "utils/sys_utils.hpp"
#include "utils/image_utils.hpp"

bool NNet::init_net(const std::string& file_path, const std::string& module_name, std::vector<std::string> layer_names, std::vector<float> channel_means, int compute_unit_preference, int execution_data_type_preference, NNetInitCallback cb){
    //this->output_layer_names = std::vector<std::string>(layer_names);
	// NNetInitCallback cb = NULL;
	this->compute_unit = compute_unit_preference;
	
	int start_time = CURRENT_TIME;
	std::lock_guard<std::mutex> guard(init_mutex);
	LOGV("VZ Debug: NNet Constructor called for %s, %s", file_path.c_str(), module_name.c_str());
	open_status = false;
	execute_status = false;
	this->module_name = module_name;
	target_channels = 3; //default, gets changed in 'set_channel_means()'
	
	for(int ch = 0; ch < channel_means.size(); ch++) {
	  	LOGV("init_net Mean for channel %d is %f", ch, channel_means[ch]);
	}

	if(module_name.find("ud_classifier") != std::string::npos)
	{
		this->classifier_op_name = layer_names[0];
	}

	//layer_names type conversion from vector<std::string> to char**
    std::vector<char*> layer_names_cstr;
    for(auto& layer_name: layer_names) {
    	layer_names_cstr.push_back(const_cast<char*> (layer_name.c_str()));
    }
    char** layer_names_ptr = NULL;
    if(!layer_names_cstr.empty()) {
    	layer_names_ptr = &layer_names_cstr[0];
    }
	input_image_cnt = 0;
	//function pointers of dl-interface
    dlinterface_lib_handle = get_dlinterface_lib_handle();
    auto DLIWrapper_create_ptr = get_create_fptr(dlinterface_lib_handle);
    auto DLIWrapper_openDLModel_ptr = get_open_fptr(dlinterface_lib_handle);
    auto DLIWrapper_openDLModel_ptr_direct = get_open_fptr_direct(dlinterface_lib_handle);
    auto DLIWrapper_get_inputDimens_fptr = get_inputDimens_fptr(dlinterface_lib_handle);
    auto DLIWrapper_get_inputChannels_fptr = get_inputChannels_fptr(dlinterface_lib_handle);
	auto DLIWrapper_openDLModel_ptr_snapOptions = get_open_fptr_snap_options(dlinterface_lib_handle);
    dli_handle = DLIWrapper_create_ptr();
	bool status = (dli_handle != NULL);
	//open model
    if(status) {
		// if(module_name == "segmenter" && (file_path.find(std::string("scene_")) == std::string::npos)){
		// 	set_channel_means({104, 117, 123, 123});
		// 	target_channels = 4;
		// }
		//set_channel_means(channel_means);
		try {
	    	// status = DLIWrapper_openDLModel_ptr_direct(dli_handle, file_path.c_str(), layer_names_ptr, layer_names_cstr.size(), compute_unit_preference, execution_data_type_preference);
			if(DLIWrapper_openDLModel_ptr_snapOptions) 
			{
				LG<<"VZ Debug: calling DLIWrapper_openDLModel_ptr_snapOptions";
				set_snap_options(file_path, module_name, layer_names, channel_means, compute_unit_preference, execution_data_type_preference);
				status = DLIWrapper_openDLModel_ptr_snapOptions(dli_handle, file_path.c_str(), snap_options);
				if(!status) {
					LG<<"VZ Debug: failed to open models using snap options api";
				}
			}
			else
			{
				LG<<"VZ Debug: failed to call DLIWrapper_openDLModel_ptr_snapOptions";
				status = DLIWrapper_openDLModel_ptr_direct(dli_handle, file_path.c_str(), layer_names_ptr, layer_names_cstr.size(), compute_unit_preference, execution_data_type_preference);
			}
			
		}
		catch(std::exception& e) {
			status = false;
			LOGV("DLInterface: SNAP session returned NULL ! Exception thrown in open for module %s | %s", module_name.c_str(), e.what());
		}
	}
	if(status) {
		// status = DLIWrapper_get_inputDimens_fptr(dli_handle, target_width, target_height);
		status = DLIWrapper_get_inputDimens_fptr(dli_handle, target_width, target_height);// &&
			//DLIWrapper_get_inputChannels_fptr(dli_handle, target_channels);			
	}
	LOGV("VZ Debug: NNet Contructor Model Dimensions - width=[%d], height=[%d]", target_width, target_height);
	LOGV("VZ Debug: NNet Contructor %s for module - %s", status?"completed":"failed", module_name.c_str());

	int end_time = CURRENT_TIME;
	LOGV("VZ Debug: engine-compute: Open time taken: %d ms for module - %s", (end_time - start_time), module_name.c_str());

	open_status = status;
	if(cb!=NULL){
	 	cb(open_status, module_name);
	}	
	return open_status;
}

NNet::NNet(const std::string& file_path, const std::string& module_name, std::vector<std::string> layer_names, std::vector<float> channel_means, int compute_unit_preference, int execution_data_type_preference, NNetInitCallback cb, bool& status)
:dli_handle(nullptr), dlinterface_lib_handle(nullptr), target_width(384), target_height(384), target_channels(3),
execute_status(false), compute_unit(SNAPInterface::SNAPCompUnit::SNAP_GPU){
	//init_net(file_path, module_name, layer_names, channel_means, compute_unit_preference, execution_data_type_preference);
	if(cb == NULL){
		LOGV("VZ Debug: NNet Constructor called without callback");
		status = init_net(file_path, module_name, layer_names, channel_means, compute_unit_preference, execution_data_type_preference, NULL);
	}
	else{
		LOGV("VZ Debug: NNet Constructor called with callback");
		init_thread = std::move(std::thread(&NNet::init_net, this, file_path, module_name, layer_names, channel_means, compute_unit_preference, execution_data_type_preference,cb));
	}
}

//  Used for TVM model
void NNet::set_snap_options(const std::string& file_path, 
					const std::string& module_name,
					std::vector<std::string> layer_names, 
                    std::vector<float> channel_means, 
                    int compute_unit_preference, 
                    int execution_data_type_preference)
{
	snap_options.compUnit = (SNAPInterface::SNAPCompUnit)compute_unit_preference;
	snap_options.cpuThreadCount = 1;
	snap_options.execType = (SNAPInterface::SNAPExecType)execution_data_type_preference;
	snap_options.model_file = file_path;
	snap_options.model_name = module_name;
	std::string model_extention = file_path.substr(file_path.rfind("."));
	LG<<"VZ Debug: set_snap_options model_extention= " << model_extention;
	std::map<std::string,SNAPInterface::SNAPModelType> mtype_map;
	mtype_map[".dlc"] = SNAPInterface::SNAPModelType::SNAP_SNPE;
	if(model_extention == ".dla" && (SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_NPU){
		LG<<"VZ Debug: set_snap_options setting the model type to MNP";
		mtype_map[".dla"] = SNAPInterface::SNAPModelType::SNAP_MNP;
	}else if(model_extention == ".nnc" && (SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_NPU){
		LG<<"VZ Debug: set_snap_options setting the model type to EDEN";
		mtype_map[".nnc"] = SNAPInterface::SNAPModelType::SNAP_EDEN;
	}
	else if((SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_NPU){
		LG<<"VZ Debug: set_snap_options setting the model type to EDEN";
		mtype_map[".tflite"] = SNAPInterface::SNAPModelType::SNAP_EDEN;
	}else if((SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_GPU
			|| (SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_CPU){
		LG<<"VZ Debug: set_snap_options setting the model type to SNAP_SNAPLITE";
		mtype_map[".tflite"] = SNAPInterface::SNAPModelType::SNAP_SNAPLITE;
	}
	mtype_map[".caffemodel"] = SNAPInterface::SNAPModelType::SNAP_CAFFE;
	mtype_map[".onnx"] = SNAPInterface::SNAPModelType::SNAP_ONNX;


	snap_options.mType = mtype_map[model_extention];
	if((SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_GPU 
			|| (SNAPInterface::SNAPCompUnit)compute_unit_preference == SNAPInterface::SNAPCompUnit::SNAP_CPU){
		snap_options.inputNames = {"inputs_0"};
	}else{
		snap_options.inputNames = {"data"};
	}
	for (int i=0;i<layer_names.size();i++)
		LG<<"VZ Debug: set_snap_options layername= " << layer_names[i];
	snap_options.outputNames = layer_names;
	snap_options.weights_file = file_path;
	//snap_options.mean = {0.0, 0.0, 0.0};
	//snap_options.scale = {0.0, 0.0, 0.0};
	snap_options.cacheSize = 0;
	return ;
}
// */
#ifndef SQA_ANALYSIS 
	bool NNet::set_channel_means(std::vector<float> means){
		if(means.empty()){
			return false;
		}
		target_channels = means.size();
		channel_means = means;

		auto DLIWrapper_setChannelMeans_ptr = get_setChannelMeans_fptr(dlinterface_lib_handle);
		float* means_array = &means[0];
		return DLIWrapper_setChannelMeans_ptr(dli_handle, means_array, means.size());

		/*channel_means = means;
		channel_scales = std::vector<float>(target_channels, 1);
		std::vector<float> zero_means(target_channels);
		auto DLIWrapper_setChannelMeans_ptr = get_setChannelMeans_fptr(dlinterface_lib_handle);
		float* means_array = &zero_means[0];
		return DLIWrapper_setChannelMeans_ptr(dli_handle, means_array, means.size());*/
	}
#endif

void NNet::get_target_dimen(int &t_height,int& t_width){
	t_width=target_width;
	t_height=target_height;
	return;
}

bool NNet::forward_pass(cv::Mat image, std::string& clasifer_or_detector){
	if(!open_status){
		return false;
	}
	if(image.empty()){
		LG<<"VZ Debug: Image for forward-pass cannot be empty";
		return false;
	}

	int width  = image.cols;
	int height = image.rows;
	int n_channels = image.channels();

	// Basic Image Pre-processing
	LG<<"Target image size (CHW): "<<target_channels<<", "<<target_height<<", "<<target_width;
	if(n_channels != target_channels){
		LG<<"VZ Debug: Expected "<<target_channels<<" channels in the input image, but received input image has "<<n_channels<<" channels";
		return false;
	}
	if(width != target_width && height != target_height){
		LG<<"VZ Debug: Miss-match in the input-image-size"<< "("<<width<<", "<<height<<")"<<" and the target-image-size"<< "("<<target_width<<", "<<target_height<<")"<<" in forward-pass";
		LG<<"VZ Debug: Resizing the image to match target input image size";
		cv::resize(image, image, cv::Size(target_width, target_height));
		//cv::resize(image, image, cv::Size(target_width, target_height), 0,0,cv::INTER_AREA);
	}
	cv::Mat target_image;
	if(clasifer_or_detector == "detector"){
		image.convertTo(target_image, CV_MAKE_TYPE(CV_32F, n_channels));
		if(this->compute_unit != 3){
			LOGV("VZ Debug: Substracting channels means 128.0, 128.0, 128.0 in library");
			target_image = (target_image - cv::Scalar(128.0, 128.0, 128.0)) / 1.0;
		}
	}else{
		target_image = normalize_image(image, this -> channel_means, this -> channel_scales);
	}
	{
        LG << "dump original boxes";
          char buffer[100];
          sprintf(buffer, "/sdcard/Download/UD_Dumps/%d_input_NNet.jpg", original_image_cnt++);
          //sprintf(buffer, "/sdcard/camera5/%d.jpg", detectorImageCounter);
          imwrite(buffer, target_image);
          if(-1 == chmod(buffer,0777)){
          LG << "no chmod";
          }
              
      }
	float* image_buffer = (float*)target_image.data;
	if(!image_buffer){
        LG<<"VZ Debug: Image buffer for forward-pass cannot be empty";
        return false;
	}	
	// Execute model on image
	int dli_image_format = (n_channels == 4) ? 2 : 1;
	bool ret_value = false;

	int start_time = CURRENT_TIME;
	auto DLIWrapper_execute_ptr = get_execute_fptr(dlinterface_lib_handle);
	if(open_status && DLIWrapper_execute_ptr (dli_handle, image_buffer, target_width, target_height, dli_image_format)) {
		execute_status = true;
		ret_value = true;
    }
    else {
    	execute_status = false;
    	ret_value = false;
    }
    LOGV("VZ Debug: NNet forward pass completed");
	int end_time = CURRENT_TIME;
	LOGV("VZ Debug: engine-compute: Execute time taken: %d ms for %s-module", (end_time - start_time), module_name.c_str());
    return ret_value;
}

float* NNet::get_layer_output(const char* layerName, int &b, int &c, int &h, int &w) {
	if(!open_status){
		return NULL;
	}
	LOGV("VZ Debug: NNet get layer output called for %s", layerName);
	auto DLIWrapper_getLayerOutput_ptr = get_output_fptr(dlinterface_lib_handle);
	float* layer_output = NULL;
	if(execute_status) {
		layer_output = DLIWrapper_getLayerOutput_ptr(dli_handle, layerName, b, c, h, w);
		LOGV("VZ Debug: NNet layer output dimensions for %s are %d, %d, %d, %d", layerName, b, c, h, w);
	}
	if(!layer_output) {
		LOGV("VZ Debug: NNet get layer output NULL for %s", layerName);
		return layer_output;
		//throw std::runtime_error("DLInterface: SNAP returned NULL for layerName " + std::string(layerName));
	}
	LOGV("VZ Debug: NNet get layer output completed for %s", layerName);
	return layer_output;
}

float* NNet::get_layer_output_with_format(const char* layerName, int &b, int &c, int &h, int &w, int &data_format) {
	if(!open_status){
		return NULL;
	}
	LOGV("VZ Debug: NNet get layer output with format called for %s", layerName);
	auto DLIWrapper_getLayerOutputAndFormat_ptr = get_output_and_format_fptr(dlinterface_lib_handle);
	float* layer_output = NULL;
	if(execute_status) {
		layer_output = DLIWrapper_getLayerOutputAndFormat_ptr(dli_handle, layerName, b, c, h, w, data_format);
		if(data_format == 1){ //nchw values are swapped in dlinterface
			int t_c = c, t_h = h, t_w = w;
			c = t_h;
			h = t_w;
			w = t_c;
		}
		LOGV("VZ Debug: NNet layer output dimensions for %s are %d, %d, %d, %d and format: %d", layerName, b, c, h, w, data_format);
	}
	if(!layer_output) {
		LOGV("VZ Debug: NNet get layer output with format NULL for %s", layerName);
		return layer_output;
		//throw std::runtime_error("DLInterface: SNAP returned NULL for layerName " + std::string(layerName));
	}
	LOGV("VZ Debug: NNet get layer output with format completed for %s", layerName);
	return layer_output;
}

float* NNet::get_layer_output_shape(const char* layerName, std::vector<int>& output_shape) {
	if(!open_status){
		return NULL;
	}
	LOGV("VZ Debug: NNet get layer output shape called for %s", layerName);
	auto DLIWrapper_getLayerOutputShape_ptr = get_output_shape_fptr(dlinterface_lib_handle);
	float* layer_output = NULL;
	if(execute_status) {
		layer_output = DLIWrapper_getLayerOutputShape_ptr(dli_handle, layerName, output_shape);
		LOGV("VZ Debug: NNet layer complete output size for %s are %d, %d, %d", layerName, output_shape[0], output_shape[1], output_shape[2]);
	}
	if(!layer_output) {
		LOGV("VZ Debug: NNet get layer output NULL for %s", layerName);
		throw std::runtime_error("DLInterface: SNAP returned NULL for layerName " + std::string(layerName));
	}
	LOGV("VZ Debug: NNet get layer output completed for %s", layerName);
	return layer_output;
}

/*
bool NNet::get_open_status(){
	return this->open_status;
}
*/

NNet::~NNet() {
	int start_time = CURRENT_TIME;
	std::lock_guard<std::mutex> guard(deinit_mutex);
	
	if(init_thread.joinable()){
		LOGV("VZ Debug: Joining init thread in ~NNet for %s",this->module_name.c_str());
		init_thread.join();
	}
	auto DLIWrapper_destroy_ptr = get_destroy_fptr(dlinterface_lib_handle);
	DLIWrapper_destroy_ptr(dli_handle);
	dlclose(dlinterface_lib_handle);
	LOGV("VZ Debug: Closing of DLInterface handle completed in ~NNet for %s",this->module_name.c_str());
	int end_time = CURRENT_TIME;
	LOGV("VZ Debug: engine-compute: Close time taken: %d ms for module %s", (end_time - start_time), module_name.c_str());
}
