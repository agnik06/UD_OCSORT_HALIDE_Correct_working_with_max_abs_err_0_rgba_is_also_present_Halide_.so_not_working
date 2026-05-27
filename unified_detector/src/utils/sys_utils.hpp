/**
 * @file   sys_utils.hpp
 * @brief  This file implements the sysutils
 *
 * This file implements the utility api's
 */
#ifndef SYS_UTILS
#define SYS_UTILS
#include <string>
#include <map>
#include <snap/snap_interface.h>
#include <core/model_config.hpp>

/**
 * @brief utility to get the model properties
 * @details utility to get the model properties
 *
 * @param[in] module name
 * @param[in] base folder location
 *
 * @return map with model name
 */
 std::vector<std::map<std::string, std::string>> get_model_properties(std::string module_info_path);

/**
 * @brief utility to check if file exists in file path
 * @details utility to check if file exists in file path
 *
 * @param[in] file path path
 *
 * @return true if file exists, false otherwise
 */
bool file_exists(std::string file_path);
//std::string get_final_models_base_path(std::string base_path, std::string module_name);
std::tuple<std::string, std::string, std::vector<std::string>, std::vector<float>, int, int,model::Version>get_snap_config(std::map<std::string, std::string> config_map, std::string temp_module_name, model::type type);

#endif