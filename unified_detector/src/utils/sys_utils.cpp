/**
 * @file   sys_utils.cpp
 * @brief  This file implements the system utilities Interface
 *
 * This file implements system utilities Interface to be used by end api's
 */
#include "sys_utils.hpp"

#ifdef __android__
#include <sys/system_properties.h>
#endif
#include "core/logger/logger.hpp"
#include <fstream>
#include <sys/stat.h>



// Function to trim whitespace from the start and end of a string


std::string trim(const std::string& str) {
   size_t first = str.find_first_not_of(" \t\n\r");
   size_t last = str.find_last_not_of(" \t\n\r");
   return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}


// Function to parse a line and extract the "key": "value" pairs manually


std::pair<std::string, std::string> parseLine(const std::string& line) {
   std::string key, value;
   size_t key_start = line.find('"'); // Find the first `"`
   if (key_start == std::string::npos) return {"", ""};
   size_t key_end = line.find('"', key_start + 1); // Find the closing `"`
   if (key_end == std::string::npos) return {"", ""};
   key = line.substr(key_start + 1, key_end - key_start - 1); // Extract key
   size_t value_start = line.find('"', key_end + 1); // Find the next `"`
   if (value_start == std::string::npos) return {"", ""};
   size_t value_end = line.find('"', value_start + 1); // Find the closing `"`
   if (value_end == std::string::npos) return {"", ""};
   value = line.substr(value_start + 1, value_end - value_start - 1); // Extract value
   return {key, value};
}


std::vector<std::map<std::string, std::string>> get_model_properties(std::string info_file_path){
		
	std::string version_id = "V1";
	std::string model_type = "cpu";
	std::string precision = "float16";
	std::string model_path = "";

	//nlohmann::json json_data;
	std::string meta_filename = info_file_path;
	size_t last_slash_pos = info_file_path.find_last_of('/');
	std::string base_folder = info_file_path.substr(0, last_slash_pos);

	std::ifstream infile(meta_filename);
	std::vector<std::map<std::string, std::string>> config_map;
    std::map<std::string, std::string> current_map;
    std::string line;
   //LOGV("aaaaaaaaaaaa start");
    try{
 	if(!base_folder.empty() && infile.good()){
 	LOGV("VZ Debug: Loading model properties from model.info file for %s",meta_filename.c_str());
    while (std::getline(infile, line)) {
       // Trim the line
       std::string trim_line = trim(line);
       // Check for the start of a new JSON object
       if (trim_line == "{") {
           current_map.clear();
       } 
       // Check for the end of a JSON object
       else if (trim_line == "}," || trim_line == "}") {
           if (!current_map.empty()) {
               config_map.push_back(current_map);
           }
       } 
       // Parse key-value pairs
       else {
           auto keyValue = parseLine(trim_line);
           if (!keyValue.first.empty()) {
               current_map[keyValue.first] = keyValue.second;
           }
       }
 	}
	LOGV("VZ Debug: Finished reading info file %s",meta_filename.c_str());
	infile.close();
}else{
	LOGV("VZ Debug: Loading model properties from model for %s info file failed, default model properties: %s, %s",meta_filename.c_str(), model_type.c_str(), precision.c_str());
	current_map["version_id"] = version_id; //Temporary placeholder to modify later with actual version id
	current_map["model_type"] = model_type;
	current_map["precision"] = precision;
	current_map["model_path"] = model_path;
}
}catch(std::exception e){
	LOGV("VZ Debug: Could not read model.info file: %s due to exception %s",meta_filename.c_str(), e.what());
}
	return config_map;
}

/**
 * @brief utility to check if file exists in file path
 * @details utility to check if file exists in file path
 *
 * @param[in] file path path
 *
 * @return true if file exists, false otherwise
 */
bool file_exists(std::string file_path){
	struct stat info;
	if(stat(file_path.c_str(), &info) == 0){
		return true;
	}
	std::ifstream infile(file_path.c_str());
	return infile.good();
}
/**
 * @brief get the model path based on base path, modulename, compute unit
 * @details get the model path based on base path, modulename, compute unit
 *
 * @param[in] base path
 * @param[in] module_name of model
 * @param[in] compute unit
 *
 * @return model path as string
 */
//base path contains 'segmenter', 'detector', etc., directories
/*std::string get_final_models_base_path(std::string base_path, std::string module_name){
	std::string vendor_path = "/vendor/etc/saiv/image_understanding/db/";
	std::string system_path = "/system/saiv/image_understanding/db/";
	std::string final_base_path = "";

    if(file_exists(base_path + module_name)){
		final_base_path = base_path;
	}
	else if(file_exists(system_path + module_name)){
    	final_base_path = system_path;
    }
	else if(file_exists(vendor_path + module_name)){
		final_base_path = vendor_path;
	}
// #ifdef TESTING
	if(final_base_path.empty()){
		LOGV("VZ Debug: Base model parent path not found for %s", module_name.c_str());
	}
	else{
		LOGV("VZ Debug: Base model parent path: %s for %s", final_base_path.c_str(), module_name.c_str());
	}
// #endif
	return final_base_path;
}*/

std::tuple<std::string, std::string, std::vector<std::string>, std::vector<float>, int, int,model::Version>get_snap_config(std::map<std::string, std::string> config_map, std::string temp_module_name, model::type type){

	snap::ComputeUnit compute_unit;
    if(config_map["model_type"] == "cpu"){
        compute_unit = snap::ComputeUnit::CPU;
    }else if(config_map["model_type"] == "gpu"){
        compute_unit = snap::ComputeUnit::GPU;
    }else if(config_map["model_type"] == "dsp"){
        compute_unit = snap::ComputeUnit::DSP;
    }else if(config_map["model_type"] == "npu"){
        compute_unit = snap::ComputeUnit::NPU;
    }else{
        LOGV("VZ Debug: Couldn't identify the COMPUTE UNIT from .info file, so setting COMPUTE UNIT to CPU");
        compute_unit = snap::ComputeUnit::CPU;
    }

    snap::ExecutionDataType execution_type;
    if(config_map["precision"] == "float16"){
        execution_type = snap::ExecutionDataType::FLOAT16;
    }else if(config_map["precision"] == "float32"){
        execution_type = snap::ExecutionDataType::FLOAT32;
    }else if(config_map["precision"] == "qasymm8"){
        execution_type = snap::ExecutionDataType::QASYMM8;
    }else if(config_map["precision"] == "qasymm16"){
        execution_type = snap::ExecutionDataType::QASYMM16;
    }else{
        LOGV("VZ Debug: Couldn't identify the EXECUTION DATA TYPE from .info file, so setting EXECUTION DATA TYPE to FLOAT16");
        execution_type = snap::ExecutionDataType::FLOAT16;
    }

    std::string model_path = config_map["model_path"];
    std::string module_name = temp_module_name;

    std::vector<std::string> output_layer_names;
    if(compute_unit == snap::ComputeUnit::GPU || compute_unit == snap::ComputeUnit::CPU){
        output_layer_names.push_back("Identity");
    }else if(type == model::type::CLASSIFIER){
		output_layer_names.push_back("prob");
	}else{
        output_layer_names.push_back("output");
    }
    std::vector<float> channel_means = {128.0,128.0,128.0};
    int compute_unit_int = static_cast<int>(compute_unit);
    int execution_type_int = static_cast<int>(execution_type);
	std::string version = config_map["version_id"];
	model::Version model_version = model::Version::V1;
	if(version == "V1"){
		model_version = model::Version::V1;
	}
	return std::tuple(model_path, module_name, output_layer_names, channel_means, compute_unit_int, execution_type_int,model_version);
}