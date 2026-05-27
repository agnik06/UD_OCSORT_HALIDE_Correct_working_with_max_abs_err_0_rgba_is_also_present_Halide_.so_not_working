/**
 * @file   unified_detector_impl.cpp
 * @brief  This file implements the UnifiedDetector Interface
 *
 * This file implements UnifiedDetector Interface to be used by end api's
 */

 #include "unified_detector_impl.hpp"
 #include "sys/stat.h"
 #include "core/logger/logger.hpp"
 #include "utils/sys_utils.hpp"
 #include "utils/image_utils.hpp"
 
 
 #define STR_HELPER(x) #x
 #define STR(x) STR_HELPER(x)
 #define UNIFIED_DETECOR_BUILD_INFO STR(__UNIFIED_DETECTOR_VERSION__)
 
 
 /**
  * @brief Constructor
  * @details Object detector constructor
  *
  * @param[in] None
  *
  * @return void
  */
 UnifiedDetectorImpl::UnifiedDetectorImpl()
    : init_cb(nullptr), m_imgWidth(0), m_imgheight(0), m_rotation(0),m_orientation(0),m_image_format(UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat::BGR),m_image_buffer(nullptr),
    m_width(0), m_height(0), detector_init_done(false),classifier_init_done(false),init_done(false),
    detector_compute_unit(static_cast<int>(snap::ComputeUnit::GPU)),classifier_compute_unit(static_cast<int>(snap::ComputeUnit::GPU)),m_mode(UNIFIED_DETECTOR_MODE::Mode::OD) {
     LOGV("VZ Debug: Using Unified ObjectDetector Built on : %s %s IST", std::string(__DATE__).c_str(), std::string(__TIME__).c_str());
     LOGV("VZ Debug: Version Info: %s",UNIFIED_DETECOR_BUILD_INFO);
     LOGV("VZ Debug: UnifiedDetector version built: %s", std::string(UNIFIED_DETECOR_BUILD_INFO).c_str());
     normal_ocsort_tracker = 0;
     zoom_ocsort_tracker = 0;
     m_frame_number =0;
     // Pre-reserve capacity to avoid per-frame vector reallocations
     result_boxes.reserve(32);
     prev_frame_pet_result_boxes.reserve(8);
     movingAvgVec.reserve(16);
 }
 UnifiedDetectorImpl::~UnifiedDetectorImpl(){
  LOGV("VZ Debug: DeInit : UnifiedDetector");
  auto start_time = CURRENT_TIME;
  std::lock_guard<std::mutex> guard_initialize(this->initialize_mtx);
  std::lock_guard<std::mutex> guard_execute(this->execute_mtx);
  std::lock_guard<std::mutex> guard_track_execute(this->execute_tracking_mtx);
  std::lock_guard<std::mutex> guard_extraction(this->result_extraction_mtx);
  reset_ocsort_tracker();
  if(classifier_snap_wrapper != NULL){
    auto deinit_start = CURRENT_TIME;
   classifier_snap_wrapper->CloseSNAP();
   DestroySribSnapWrapperSession(classifier_snap_wrapper);
   auto deinit_end = CURRENT_TIME;
   LOGV("VZ Debug: DeInit : Total time taken for model %s : %lld ms",classifier_model_name.c_str(), (deinit_end - deinit_start));
  }
  if(detector_snap_wrapper != NULL){
    auto deinit_start = CURRENT_TIME;
   detector_snap_wrapper->CloseSNAP();
   DestroySribSnapWrapperSession(detector_snap_wrapper);
   auto deinit_end = CURRENT_TIME;
   LOGV("VZ Debug: DeInit : Total time taken for model %s : %lld ms",detector_model_name.c_str(), (deinit_end - deinit_start));
  }
  auto deinit_finish = CURRENT_TIME;
  LOGV("Deleting UnifiedDetector...............");
  LOGV("VZ Debug: DeInit with close: Total time taken: %lld",deinit_finish - start_time);
 }
 void UnifiedDetectorImpl::reset_ocsort_tracker(){
    if(normal_ocsort_tracker){
        delete normal_ocsort_tracker;
        normal_ocsort_tracker =0;
    }
    if(zoom_ocsort_tracker){

        delete zoom_ocsort_tracker;
        zoom_ocsort_tracker = 0;
    }
    m_frame_number =0;
    movingAvgVec.clear();
    prev_frame_pet_result_boxes.clear();
    
 }

 void UnifiedDetectorImpl::initialize_callback(){
    impl_init_cb = [this](bool status, std::string module_name){
        LOGV("VZ Debug: Initialization callback for %s", module_name.c_str());
        if(module_name == "ud_detector"){
            if(!status){
                LOGV("VZ Debug: Initilization of detector is failed");
                this->detector_init_done = false;
            }
            else{
                this->detector_init_done = true;
            }
        }else if(module_name == "ud_classifier"){
            if(!status){
                LOGV("VZ Debug: Initilization of classifer is failed");
                this->classifier_init_done = false;
            }
            else{
                this->classifier_init_done = true;
            }
        }
        this->init_done = (this->detector_init_done && this->classifier_init_done);
        try {
            if(this->init_cb != nullptr){
            this->init_cb(this->init_done);
            }
        } catch (std::exception& ex) {
            LOGV("VZ Debug: Exception - %s", ex.what());
            this->init_done = false;
        }
    };
  }
  bool UnifiedDetectorImpl::detector_initialize(UNIFIED_DETECTOR_MODE::Mode mode, std::string model_base_path){
    std::vector<std::map<std::string, std::string>> config_map = get_model_properties(model_base_path);
    std::map<std::string, std::string> detector_config_map = config_map[0];
    LOGV("VZ Debug: setting Detector COMPUTE UNIT to %s, %s", detector_config_map["model_type"].c_str(), detector_config_map["precision"].c_str());
    auto snap_config_detector = get_snap_config(detector_config_map, "ud_detector",model::type::DETECTOR);
    detector_output_layer_names = std::get<2>(snap_config_detector);
    const std::string file_path = std::get<0>(snap_config_detector);
		const std::string module_name = std::get<1>(snap_config_detector);
    int compute_unit_preference = std::get<4>(snap_config_detector);
    int execution_data_type_preference = std::get<5>(snap_config_detector);
    this->detector_compute_unit= compute_unit_preference;
    //this->detector_net = std::make_shared<NNet>(std::get<0>(snap_config_detector), std::get<1>(snap_config_detector), std::get<2>(snap_config_detector), std::get<3>(snap_config_detector), std::get<4>(snap_config_detector), std::get<5>(snap_config_detector), impl_init_cb, status);
    struct snap::SnapOptionsV5 options;
    snap::PerfProfileLevel perfProfileLevel;
    std::string model_extention = file_path.substr(file_path.rfind("."));
    LOGV("VZ Debug: set_snap_options model_extention= %s", model_extention.c_str());
    detector_model_name = file_path.substr(file_path.rfind("/")+1);
    LOGV("VZ Debug: set_snap_options model_extention= %s", detector_model_name.c_str());
    std::map<std::string,snap::ModelType> mtype_map;
    mtype_map[".dlc"] = snap::ModelType::SNPE;
    if(model_extention == ".dla" && (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::NPU){
      LOGV("VZ Debug: set_snap_options setting the model type to MNP");
      mtype_map[".dla"] = snap::ModelType::MNP;
    }else if(model_extention == ".nnc" && (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::NPU){
      LOGV("VZ Debug: set_snap_options setting the model type to EDEN");
      mtype_map[".nnc"] = snap::ModelType::EDEN;
    }
    else if((snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::NPU){
      LOGV("VZ Debug: set_snap_options setting the model type to EDEN")
      mtype_map[".tflite"] = snap::ModelType::EDEN;
    }else if((snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::GPU
        || (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::CPU){
      LOGV("VZ Debug: set_snap_options setting the model type to SNAPLITE")
      mtype_map[".tflite"] = snap::ModelType::SNAPLITE;
    }
    mtype_map[".caffemodel"] = snap::ModelType::CAFFE;
    mtype_map[".onnx"] = snap::ModelType::ONNX;

    options.mType =  mtype_map[model_extention];
    options.compUnit = (snap::ComputeUnit)(compute_unit_preference);
    options.execType = (snap::ExecutionDataType)(execution_data_type_preference);
    options.cpuThreadCount = 1;
    options.outputNames = detector_output_layer_names;
    if((snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::GPU 
        || (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::CPU){
      options.inputNames = {"inputs_0"};
    }else{
      options.inputNames = {"data"};
    }
    options.model_file = file_path; 
    options.weights_file = file_path;
    if(options.compUnit == snap::ComputeUnit::DSP){
      perfProfileLevel.PerfProfileExec.snpeExecutePerfSetting = snap::SnpePerfProfileLevel::LOW_POWER_SAVER;
      options.profileLevel = perfProfileLevel;
      LOGV("VZ Debug: ud_detector DSP setting optionsV5.powerprofile : [%d]", snap::SnpePerfProfileLevel::LOW_POWER_SAVER);
    }else if(options.compUnit == snap::ComputeUnit::NPU){
      perfProfileLevel.PerfProfileOpen.ennInitPerfSetting = snap::EnnPerfProfileLevel::MAXENUM;
      options.profileLevel = perfProfileLevel;
      options.ofi_preset = "perfmode:5;configid:100009";
      LOGV("VZ Debug: ud_detector NPU setting optionsV5.powerprofile : [%d], ofi_preset=[%s]", snap::EnnPerfProfileLevel::MAXENUM, options.ofi_preset.c_str());
    }
    auto model_init_time = CURRENT_TIME;
    detector_snap_wrapper = CreateSribSnapWrapperSession();
    auto status = detector_snap_wrapper->OpenSNAP(
        static_cast<const struct snap::SnapOptionsV5>(options));
        auto model_init_finish = CURRENT_TIME;
      LOGV("VZ Debug: Init : Total time taken for model %s : %lld ms",detector_model_name.c_str(), (model_init_finish - model_init_time));
      if(status == snap::OK){
        //Detector Post Processor Initialization
        m_decoder = std::make_shared<Decoder>(384,384,std::get<6>(snap_config_detector));       
        std::shared_ptr<ROIDataHandler> roi_data_handler = std::make_shared<ROIDataHandler>(ROIDataFactory::get_data(std::get<6>(snap_config_detector)));
        roi_mapper = std::make_shared<DefaultROIMapper>(roi_data_handler);
        pet_roi_filterer = std::make_shared<PetROIFilterer>(roi_data_handler, mode);
        text_roi_filterer = std::make_shared<TextROIFilterer>(roi_data_handler, mode);
        return true;
      }
      return false;
  }

  bool UnifiedDetectorImpl::classifier_initialize(UNIFIED_DETECTOR_MODE::Mode mode, std::string model_base_path){
    //Classifier Intialization
    std::vector<std::map<std::string, std::string>> config_map = get_model_properties(model_base_path);
    std::map<std::string, std::string> classifer_config_map = config_map[1];
    LOGV("VZ Debug: setting Classifier COMPUTE UNIT to %s, %s", classifer_config_map["model_type"].c_str(), classifer_config_map["precision"].c_str());
    auto snap_config_classifier = get_snap_config(classifer_config_map, "ud_classifier",model::type::CLASSIFIER);
    classifier_output_layer_names = std::get<2>(snap_config_classifier);
    const std::string file_path = std::get<0>(snap_config_classifier);
		const std::string module_name = std::get<1>(snap_config_classifier);
    int compute_unit_preference = std::get<4>(snap_config_classifier);
    int execution_data_type_preference = std::get<5>(snap_config_classifier);
    this->classifier_compute_unit= compute_unit_preference;
    //this->classifier_net = std::make_shared<NNet>(std::get<0>(snap_config_classifier), std::get<1>(snap_config_classifier), std::get<2>(snap_config_classifier), std::get<3>(snap_config_classifier), std::get<4>(snap_config_classifier), std::get<5>(snap_config_classifier), impl_init_cb, status);
    struct snap::SnapOptionsV5 options;
    snap::PerfProfileLevel perfProfileLevel;
	std::string model_extention = file_path.substr(file_path.rfind("."));
  LOGV("VZ Debug: set_snap_options model_extention= %s", model_extention.c_str());
  classifier_model_name = file_path.substr(file_path.rfind("/")+1);
  LOGV("VZ Debug: set_snap_options classifier_model_name= %s", classifier_model_name.c_str());
	std::map<std::string,snap::ModelType> mtype_map;
	mtype_map[".dlc"] = snap::ModelType::SNPE;
	if(model_extention == ".dla" && (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::NPU){
    LOGV("VZ Debug: set_snap_options setting the model type to MNP");
		mtype_map[".dla"] = snap::ModelType::MNP;
	}else if(model_extention == ".nnc" && (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::NPU){
    LOGV("VZ Debug: set_snap_options setting the model type to EDEN");
		mtype_map[".nnc"] = snap::ModelType::EDEN;
	}
	else if((snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::NPU){
    LOGV("VZ Debug: set_snap_options setting the model type to EDEN");
		mtype_map[".tflite"] = snap::ModelType::EDEN;
	}else if((snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::GPU
			|| (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::CPU){
    LOGV("VZ Debug: set_snap_options setting the model type to SNAPLITE");
		mtype_map[".tflite"] = snap::ModelType::SNAPLITE;
	}
	mtype_map[".caffemodel"] = snap::ModelType::CAFFE;
	mtype_map[".onnx"] = snap::ModelType::ONNX;

  options.mType =  mtype_map[model_extention];
  options.compUnit = (snap::ComputeUnit)(compute_unit_preference);
  options.execType = (snap::ExecutionDataType)(execution_data_type_preference);
  options.cpuThreadCount = 1;
  options.outputNames = classifier_output_layer_names;
  if((snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::GPU 
			|| (snap::ComputeUnit)compute_unit_preference == snap::ComputeUnit::CPU){
    options.inputNames = {"inputs_0"};
	}else{
		options.inputNames = {"data"};
	}
  options.model_file = file_path; 
  options.weights_file = file_path;
  if(options.compUnit == snap::ComputeUnit::DSP){
    perfProfileLevel.PerfProfileExec.snpeExecutePerfSetting = snap::SnpePerfProfileLevel::LOW_POWER_SAVER;
    options.profileLevel = perfProfileLevel;
    LOGV("VZ Debug: ud_classifier DSP setting optionsV5.powerprofile : [%d]", snap::SnpePerfProfileLevel::LOW_POWER_SAVER);
  }
  auto model_init_time = CURRENT_TIME;
  classifier_snap_wrapper = CreateSribSnapWrapperSession();
  auto status = classifier_snap_wrapper->OpenSNAP(
      static_cast<const struct snap::SnapOptionsV5>(options));
      auto model_init_finish = CURRENT_TIME;
      LOGV("VZ Debug: Init : Total time taken for model %s: %lld ms",classifier_model_name.c_str(),model_init_finish - model_init_time);
    if(status == snap::OK){
      //Classifier Post Processor Initialization
      std::shared_ptr<DataHandler> classifier_data_handler = std::make_shared<DataHandler>(DataFactory::get_data(std::get<6>(snap_config_classifier))); 
      
      base_ids.push_back(classifier_data_handler->getBaseId("partial_sky"));
      auto skyscraper_base_ids = classifier_data_handler->getBaseIdsFromDisplay("Skyscraper");
      auto city_base_ids = classifier_data_handler->getBaseIdsFromDisplay("City");
      base_ids.insert(base_ids.end(), skyscraper_base_ids.begin(), skyscraper_base_ids.end());
      base_ids.insert(base_ids.end(), city_base_ids.begin(), city_base_ids.end());
      
      classifier = std::make_shared<BaseClassifier> (classifier_net);
      classifier = std::make_shared<ClassifierWithVerifiedTopCrop>(classifier, base_ids);
      classifier = std::make_shared<ClassifierWithFlip>(classifier);

      image_tag_mapper = std::make_shared<DefaultMapper>(classifier_data_handler);
      image_tag_filterer =std::make_shared<BaseClassifierFilterer>(classifier_data_handler, true); // threholding;
      return true;
    }
    return false;
  }
 
 /**
  * @brief Initialization function with mode and model base path
  * @details Initilizes teh model with mode and basepath
  *
  * @param[in] Mode
  * @param[in] Model base path
  *
  * @return bool status
  */
 bool UnifiedDetectorImpl::initialize(UNIFIED_DETECTOR_MODE::Mode mode, char* model_base_path){
     std::lock_guard<std::mutex> guard(this->initialize_mtx);
     auto start_time = CURRENT_TIME;
    #ifdef ANDROID_ARM_NEON
        LOGV("Initializing armneon used in preproc for nv21...");
    #else
        LOGV("Initializing ...");
    #endif
    LOGV("Models path : %s, init mode : %d",model_base_path,static_cast<int>(mode));
    if(file_exists(model_base_path) == false){
      LOGV("Models are not present on device");
      return false;
    }
    normal_ocsort_tracker = 0;
    zoom_ocsort_tracker = 0;
    m_frame_number = 0;
    m_mode= mode;
    bool status= false;
    //Detector Initialization
    if(mode==UNIFIED_DETECTOR_MODE::Mode::OD && this->detector_snap_wrapper == NULL){
      status=detector_initialize(mode,model_base_path);
    }
    if((mode==UNIFIED_DETECTOR_MODE::Mode::CLS || mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_SINGLE_TAKE) && this->classifier_snap_wrapper == NULL){
      status=classifier_initialize(mode,model_base_path);
    }
    if(mode == UNIFIED_DETECTOR_MODE::Mode::ODCLS || mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_REMASTER){
      if(detector_snap_wrapper == NULL){
        status=detector_initialize(mode,model_base_path);
      }
      if(this->classifier_snap_wrapper == NULL){
        status=classifier_initialize(mode,model_base_path);
      }
    }
    this->init_done = status;
    auto init_finish = CURRENT_TIME;
    LOGV("VZ Debug: Init : Total time taken: %lld ms",init_finish - start_time);
    return status;
 }
 
 /**
  * @brief Initializes teh Unified Detector
  * @details Initializes the Unified detector with mode, basepath and callback
  *
  * @param[in] takes the callback, mode and basepath
  * @param[in] takes the mode
  *
  * @return bool status
  */
 bool UnifiedDetectorImpl::initialize(void (*cb)(bool status),UNIFIED_DETECTOR_MODE::Mode mode, char* model_base_path){
    LOGV("initialize : mode [%d]",mode);
    this->init_cb=cb;
    bool status=initialize(mode,model_base_path);
    return status;    
 }

 bool confidenceComparator(unified_detector::BoundingBox box1, unified_detector::BoundingBox box2){
  return box2.score < box1.score;
 }

 bool UnifiedDetectorImpl::detector_execute(char* image_buffer, int width, int height, int image_rotation, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,UNIFIED_DETECTOR_MODE::Mode mode, bool& need_to_run_classifier, char* file_name){
  result_boxes.clear();
  int target_height=384;
  int target_width=384;
  //this->detector_net->get_target_dimen(target_width, target_height);
  #ifdef DUMPBOX //Input image with detector boxes ; need to keep tese dumps on always
  {
    cv::Mat dump_image = preprocess_image(image_buffer, width, height, width, height, 0, image_format);
    LG << "dump original image";
      char buffer[100];
      sprintf(buffer, "/data/vendor/camera/uod_original/%d.jpg", original_image_cnt++);
      //sprintf(buffer, "/sdcard/camera5/%d.jpg", detectorImageCounter);
      imwrite(buffer, dump_image);
      if(-1 == chmod(buffer,0777)){
        LG << "no chmod";
      }
                  
  }
#endif //DUMPBOX
  std::string detector = "detector";
  auto preprocess_start_time = CURRENT_TIME;

  // ------------------------------------------------------------------
  //  Fused preprocess + normalize. Runs through Halide AOT when
  //  USE_HALIDE_PREPROCESS is defined AND input is NV21/NV12 @ 384x384;
  //  falls back to OpenCV preprocess_image + normalize_image otherwise.
  //  See src/utils/halide_preprocess.cpp.
  // ------------------------------------------------------------------
  DetectorPreprocessResult prep;
  if (!preprocess_detector_input_halide(image_buffer, width, height,
                                        target_width, target_height,
                                        image_rotation, image_format, prep)) {
      LOGV("VZ Debug: detector preprocess_detector_input_halide failed");
      return false;
  }
  cv::Mat image = prep.bgr_u8;
  //bool forward_pass_status = this->detector_net->forward_pass(image, detector);
  #ifdef DUMPBOX //Input image with detector boxes ; need to keep tese dumps on always
  {
    LG << "dump preprossed image";
      char buffer[100];
      sprintf(buffer, "/data/vendor/camera/uod_preprocessed/%d.jpg", preprocess_image_cnt++);
      //sprintf(buffer, "/sdcard/camera5/%d.jpg", detectorImageCounter);
      imwrite(buffer, image);
      if(-1 == chmod(buffer,0777)){
        LG << "no chmod";
      }

  }
#endif //DUMPBOX
  cv::Mat target_image = prep.input_f32;
  std::vector<snap::DataBuffer> inputs, outputs;
  snap_utils::prepare_snap_input(target_image, inputs, 384,384,3, this->detector_compute_unit, detector);
  #ifdef TESTING
		std::string dumppath_input = "/sdcard/ud_input_raws/" + std::to_string(0) +".raw";
    float* dump_buffer = (float*)target_image.data;
			std::ofstream out_debug(dumppath_input, std::ios::out | std::ios::binary);
			if(!out_debug) {
				LG<<"VZ Debug: Cannot open file."<<dumppath_input;
			} else {
				out_debug.write((char *) dump_buffer, sizeof(float)*3*target_width*target_height);
				out_debug.close();
			}
			//input_image_cnt++;
		LG<<"VZ Debug: Dumped input success";
	#endif
  auto preprocess_end_time = CURRENT_TIME;
  LOGV("VZ Debug: preprocess : Total time taken for detector: %lld ms, frame_number: %d",preprocess_end_time - preprocess_start_time, m_frame_number);
  auto model_execute_start_time = CURRENT_TIME;
  // passing the inputs as vector, getting the outputs as vector
  detector_snap_wrapper->ExecuteSNAP(inputs, &outputs);  // input is the prepared image data buffer for SNAP inference done by the prepare_snap_input function
  auto model_execute_end_time = CURRENT_TIME;
  LOGV("VZ Debug: Execute : Total time taken for model %s: %lld ms",detector_model_name.c_str(), model_execute_end_time - model_execute_start_time);
  /*if(!forward_pass_status)
  {
      LG<<"VZ Debug: unified_detector Forward Pass Failed";
      return false;
  }*/
	int n, c, h, w, data_format,temp;
  snap_utils::get_output_dimensions(outputs,n,c,h,w,detector);
  if(outputs[0].dataFormat == snap::DataFormat::NCHW){
    temp = h;
    h = c;
    c=w;
    w= temp;
    LOGV("SNAP Output shape for detector is [%s] and dimensions are changed to NHWC for post processing n :[%d], h :[%d], w :[%d], c:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,h,w,c);
  }else{
    LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], h :[%d], w :[%d], c:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,h,w,c);
  }
  float *layer_output_flattened = static_cast<float *>(outputs[0].data);
  //Dumping detector output raws
  #ifdef DUMPBOX
    bool result_dets = detector_utils::dump_output_raws(layer_output_flattened, file_name, n, h, w, c);
  #endif
  //std::vector<int> output_shape;
  //layer_output_flattened = this->detector_net->get_layer_output_shape(detector_output_layer_names[0].c_str(), output_shape);
  Tensor2D<float> output_tensor(layer_output_flattened, h, w);
  std::vector<std::vector<float> > final_boxes;
	final_boxes.clear();
	final_boxes = m_decoder->decode(output_tensor, m_decoder->score_threshold, m_decoder->nms_threshold);
  std::vector<unified_detector::BoundingBox> boxes = detector_utils::get_boxes_from_2D_tensors(final_boxes, image);
  result_boxes = boxes;
  if(!boxes.empty()){
    result_boxes = this->roi_mapper->map(result_boxes);
    result_boxes = this->pet_roi_filterer->filter(result_boxes, image,mode);
    //LOGV("VZ DEBUG: Size of boxes after pet filterer: [%d]", result_boxes.size());
    result_boxes = this->text_roi_filterer->filter(result_boxes, image,mode);
    //LOGV("VZ DEBUG: Size of boxes after text filterer: [%d]", result_boxes.size());
    if(mode == UNIFIED_DETECTOR_MODE::Mode::ODCLS){
      bool od_classes_detected = detector_utils::od_classes_detected(result_boxes);
      if(!od_classes_detected){
        need_to_run_classifier = true;
      }else{
        need_to_run_classifier = false;
        result_boxes = detector_utils::remove_object_boxes(result_boxes);
        result_boxes = detector_utils::correct_roi_boxes(result_boxes, target_width,target_height,image_rotation,width,height);
      }
    }else{
      result_boxes = detector_utils::correct_roi_boxes(result_boxes, target_width,target_height,image_rotation,width,height);
    }
  }else{
      result_boxes.clear();
      if(mode == UNIFIED_DETECTOR_MODE::Mode::ODCLS){
        need_to_run_classifier = true;
       }
      LOGV("No boxes found returning false");
      return false;
  }
  return true;
 }

 bool UnifiedDetectorImpl::classifier_execute(char* image_buffer, int width, int height, int image_rotation, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,UNIFIED_DETECTOR_MODE::Mode mode, char* file_name){
  int target_height=384;
  int target_width=384;
  #ifdef DUMPBOX //Input image with detector boxes ; need to keep tese dumps on always
    {
      cv::Mat dump_image = preprocess_image(image_buffer, width, height, width, height, 0, image_format);
      LG << "dump original image";
        char buffer[100];
        sprintf(buffer, "/data/vendor/camera/uod_original/%d.jpg", original_image_cnt++);
        //sprintf(buffer, "/sdcard/camera5/%d.jpg", detectorImageCounter);
        imwrite(buffer, dump_image);
        if(-1 == chmod(buffer,0777)){
          LG << "no chmod";
        }
                    
    }
  #endif //DUMPBOX
  //this->classifier_net->get_target_dimen(target_width, target_height);
  std::string classifier = "classifier";
  auto preprocess_start_time = CURRENT_TIME;
  cv::Mat image = preprocess_image(image_buffer, width, height, target_width, target_height, image_rotation, image_format);
  #ifdef DUMPBOX //Input image with detector boxes ; need to keep tese dumps on always
    {
      LG << "dump preprossed image";
        char buffer[100];
        sprintf(buffer, "/data/vendor/camera/uod_preprocessed/%d.jpg", preprocess_image_cnt++);
        //sprintf(buffer, "/sdcard/camera5/%d.jpg", detectorImageCounter);
        imwrite(buffer, image);
        if(-1 == chmod(buffer,0777)){
          LG << "no chmod";
        }
                    
    }
  #endif //DUMPBOX
  cv::Mat preprocessed_image;
  cv::Mat roi_image;
  
  //cropResizeBGR((char*)image.data, target_width, target_height, 0, 0, target_width-1, target_height-1, target_width, target_height, preprocessed_image);
  cv::Mat target_image;
	target_image = normalize_image(image, {128.0, 128.0, 128.0}, {1.0, 1.0, 1.0});
	
  std::vector<snap::DataBuffer> inputs, outputs;

  snap_utils::prepare_snap_input(target_image, inputs, 384,384,3, this->classifier_compute_unit, classifier);
  auto preprocess_end_time = CURRENT_TIME;
  LOGV("VZ Debug: preprocess : Total time taken for classifier: %lld ms",preprocess_end_time - preprocess_start_time);
  auto model_execute_start_time = CURRENT_TIME;
  classifier_snap_wrapper->ExecuteSNAP(inputs, &outputs);
  auto model_execute_end_time = CURRENT_TIME;
  LOGV("VZ Debug: Execute : Total time taken for model %s: %lld ms", classifier_model_name.c_str(), model_execute_end_time - model_execute_start_time);
  int n, c, h, w, data_format;
  snap_utils::get_output_dimensions(outputs,n,c,h,w,classifier);
  if(outputs[0].dataFormat == snap::DataFormat::NCHW){
    LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], c :[%d], h :[%d], w:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,c,h,w);
  }else{
    LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], h :[%d], w :[%d], c:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,h,w,c);
  }
  float *layer_output_flattened = static_cast<float *>(outputs[0].data);
  int op_size = n * c * h * w;
  std::vector<float> softmax(layer_output_flattened, layer_output_flattened + op_size);
  //Dumping classifier output raws
  #ifdef DUMPBOX
    bool result_cls = classifier_utils::dump_output_raws(softmax, file_name); 
  #endif
  std::vector<float> features = classifier_utils::apply_custom_softmax(softmax);
  int top_baseline_id = std::distance(features.begin(), std::max_element(features.begin(), features.end()));
  if (std::find(base_ids.begin(), base_ids.end(), top_baseline_id) != base_ids.end()) {
    cv::Mat image_top_crop = classifier_utils::get_top_cropped(image);
    char* crop_image_buffer = (char*)image_top_crop.data;
    int crop_width = image_top_crop.cols;
    int crop_height = image_top_crop.rows;
    cv::Mat target_image;
    cv::Mat crop_preprocess_image;
    cropResizeBGR(crop_image_buffer, crop_width, crop_height, 0, 0, crop_width-1, crop_height-1, target_width, target_height, crop_preprocess_image);
    crop_preprocess_image.convertTo(target_image, CV_MAKE_TYPE(CV_32F, 3));
    target_image = (target_image - cv::Scalar(128.0, 128.0, 128.0)) / 1.0;
    std::vector<snap::DataBuffer> inputs, outputs;

    snap_utils::prepare_snap_input(target_image, inputs, 384,384,3, this->classifier_compute_unit, classifier);
    auto model_execute_start_time = CURRENT_TIME;
    classifier_snap_wrapper->ExecuteSNAP(inputs, &outputs);
    auto model_execute_end_time = CURRENT_TIME;
    LOGV("VZ Debug: Execute : Total time taken for model %s: %lld ms",classifier_model_name.c_str(),model_execute_end_time - model_execute_start_time);
    snap_utils::get_output_dimensions(outputs,n,c,h,w,classifier);
    if(outputs[0].dataFormat == snap::DataFormat::NCHW){
      LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], c :[%d], h :[%d], w:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,c,h,w);
    }else{
      LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], h :[%d], w :[%d], c:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,h,w,c);
    }
    float *layer_output_flattened = static_cast<float *>(outputs[0].data);
    op_size = n * c * h * w;
    std::vector<float> softmax(layer_output_flattened, layer_output_flattened + op_size);
    features = classifier_utils::apply_custom_softmax(softmax);
  }
  /*std::vector<float> features_det_crop;
  if(!result_boxes.empty()){
    for(auto entity : result_boxes){
      if(entity.tag_id == 1){
        LOGV("Executing claasifier with OD object box crop and box is %s ",entity.tag.c_str());
        unified_detector::Rectangle roi_loc = entity.location;
        classifier_utils::get_expanded_image_crop(image, roi_loc, roi_image);
        cv::Mat target_image;
        roi_image.convertTo(target_image, CV_MAKE_TYPE(CV_32F, 3));
        target_image = normalize_image(roi_image, {128.0,128.0,128.0}, {1.0,1.0,1.0});
        std::vector<snap::DataBuffer> inputs, outputs;
        struct snap::DataBuffer snap_input;

        snap_input.data = target_image.data; // this is the data that is input to Snap execute
        snap_input.shape = {1, 384, 384, 3};
        snap_input.dataType = snap::DataBufferType::FLOAT32;
        snap_input.dataFormat = snap::DataFormat::NHWC;
        inputs.push_back(snap_input);
        classifier_snap_wrapper->ExecuteSNAP(inputs, &outputs);
        for (int i = 0; i < outputs[0].shape.size(); ++i) {
          LOGV("VZ Debug : classifier_output_shape[%d] = %d", i,
                              outputs[0].shape[i]);
        }
        c = outputs[0].shape[0]; h = outputs[0].shape[1]; w = outputs[0].shape[2]; b = outputs[0].shape[3];
        float *layer_output_flattened = static_cast<float *>(outputs[0].data);
        std::vector<float> softmax(layer_output_flattened, layer_output_flattened + op_size);
        features = classifier_utils::apply_custom_softmax(softmax);
      }
    }
  }
  features.insert(features.end(), features_det_crop.begin(), features_det_crop.end());*/
  std::vector<Entity> tagged_entities;
  if(!features.empty()){
    
    tagged_entities = image_tag_mapper->map(features);
    if(image_tag_filterer != nullptr){
      tagged_entities = image_tag_filterer->filter(tagged_entities);
      LOGV("CD Debug: output size after filterer:%zu",tagged_entities.size());
    }
  }else{
    return false;
  }
  classifier_utils::assign_scene_category(tagged_entities);
  tagged_entities = classifier_utils::remove_unsupported_classes(tagged_entities);
  //Disbaled as per the Kim's request
  if(mode != UNIFIED_DETECTOR_MODE::Mode::CUSTOM_REMASTER){
    result_boxes.clear();
  }
  for (auto& e : tagged_entities) {
    if(static_cast<int>(100.0 * e.display_tag_score) > 0){
      unified_detector::BoundingBox new_box({0,0,0,0},
        {width, height},
        (char*)e.display_tag.c_str(), static_cast<int>(100.0 * e.display_tag_score),static_cast<int>(e.scene_category),-1,true, false);
      result_boxes.emplace_back(new_box);
    }
  }
  if(!result_boxes.empty())
      sort(result_boxes.begin(),result_boxes.end(), &confidenceComparator);
  return true;
}

bool UnifiedDetectorImpl::execute(char* image_buffer, int width, int height, int image_rotation, UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format, bool use_similarity,UNIFIED_DETECTOR_MODE::Mode mode, UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType zoom_type, char* file_name){
  std::lock_guard<std::mutex> guard(this->execute_mtx);
  // m_frame_number++;
  auto start_time = CURRENT_TIME;
  bool status= false;
  if(mode==UNIFIED_DETECTOR_MODE::Mode::OD && this->detector_snap_wrapper == NULL){
   status=detector_initialize(mode,model_base_path);
  }
  if((mode==UNIFIED_DETECTOR_MODE::Mode::CLS || mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_SINGLE_TAKE) && this->classifier_snap_wrapper == NULL){
    status=classifier_initialize(mode,model_base_path);
  }
  if(mode == UNIFIED_DETECTOR_MODE::Mode::ODCLS || mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_REMASTER){
    if(this->detector_snap_wrapper == NULL){
      status=detector_initialize(mode,model_base_path);
    }
    if(this->classifier_snap_wrapper == NULL){
      status=classifier_initialize(mode,model_base_path);
    }
  }
  LG<<"execute: image_rotation : ["<<image_rotation<<"], width : ["<<width<<"], height : ["<<height<<"], image_format: ["<<image_format<<"], mode : ["<<static_cast<int>(mode)<<"]";
  if (!this->init_done) {
      LOGV("VZ Debug: Execute called without proper init completion. Execute aborted !");
      return false;
  }
  else if(width <=0 || height <= 0){
    LOGV("Image meta data is not proper.. image width : %d, image height : %d",width,height);
    return false;
  }
  m_image_buffer = image_buffer;
  m_width = width;
  m_height = height;
  m_mode = mode;
  cv::Mat image;
  bool execute_status = false;
  bool need_to_run_classifier = false;
  result_boxes.clear();
  if(mode == UNIFIED_DETECTOR_MODE::Mode::OD){
     execute_status = detector_execute(image_buffer, width, height, image_rotation, image_format,mode, need_to_run_classifier, file_name);
  }else if(mode == UNIFIED_DETECTOR_MODE::Mode::CLS || mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_SINGLE_TAKE){
     execute_status = classifier_execute(image_buffer, width, height, image_rotation, image_format,mode, file_name);
  }else if(mode == UNIFIED_DETECTOR_MODE::Mode::ODCLS || mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_REMASTER){
     execute_status = detector_execute(image_buffer, width, height, image_rotation, image_format,mode, need_to_run_classifier, file_name);
     //This is for kim request to rin both detector and classifier
     //execute_status = classifier_execute(image_buffer, width, height, image_rotation, image_format,mode);
     if(mode == UNIFIED_DETECTOR_MODE::Mode::CUSTOM_REMASTER || need_to_run_classifier){
       execute_status = classifier_execute(image_buffer, width, height, image_rotation, image_format,mode, file_name);
     }
  }
  auto execute_finish = CURRENT_TIME;
  LOGV("VZ Debug: Execute : Total time taken: %lld ms",execute_finish - start_time);
  return execute_status;
}

bool UnifiedDetectorImpl::execute(UNIFIED_DETECTOR_MODE::Mode mode, char* data, int width,int height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,char *file_name){
  if (!this->init_done) {
    LOGV("VZ Debug: Execute called without proper init completion. Execute aborted !");
    return false;
  }else if(width <=0 || height <= 0){
    LOGV("Image meta data is not proper.. image width : %d, image height : %d",width,height);
    return false;
  }
  cv::Mat image(height, width, CV_8UC3, data);
  LOGV("execute called with width: %d and height: %d image_rotation : %d, mode : %d",width,height,image_rotation,static_cast<int>(mode));
  auto start_time = CURRENT_TIME;
  int target_height =384;
  int target_width = 384;
  if(mode == UNIFIED_DETECTOR_MODE::Mode::OD){
    //this->detector_net->get_target_dimen(target_width, target_height);
    LOGV("execute called with target_width: %d and target_height: %d",target_width,target_height);
    cv::Mat target_image;
    cv::resize(image, image, cv::Size(target_width, target_height));
    image.convertTo(target_image, CV_MAKE_TYPE(CV_32F, 3));
		if(this->detector_compute_unit != 3){
			//LOGV("VZ Debug: Substracting channels means 128.0, 128.0, 128.0 in library");
			target_image = (target_image - cv::Scalar(128.0, 128.0, 128.0)) / 1.0;
		}
    std::vector<snap::DataBuffer> inputs, outputs;
    std::string detector = "detector";
    snap_utils::prepare_snap_input(target_image, inputs, 384,384,3, this->detector_compute_unit, detector);
    auto model_execute_start_time = CURRENT_TIME;
    // passing the inputs as vector, getting the outputs as vector
    detector_snap_wrapper->ExecuteSNAP(inputs, &outputs);
    auto model_execute_end_time = CURRENT_TIME;
    LOGV("VZ Debug: Execute : Total time taken for model %s: %lld ms",detector_model_name.c_str(), model_execute_end_time - model_execute_start_time);
	int n, c, h, w, data_format;
  snap_utils::get_output_dimensions(outputs,n,c,h,w,detector);
  if(outputs[0].dataFormat == snap::DataFormat::NCHW){
    LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], c :[%d], h :[%d], w:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,c,h,w);
  }else{
    LOGV("SNAP Output shape for detector is [%s] and dimensions are n :[%d], h :[%d], w :[%d], c:[%d]", outputs[0].dataFormat==snap::DataFormat::NHWC ? "NHWC":"NCHW", n,h,w,c);
  }
  float *layer_output_flattened = static_cast<float *>(outputs[0].data);
  //std::vector<int> output_shape;
  //layer_output_flattened = this->detector_net->get_layer_output_shape(detector_output_layer_names[0].c_str(), output_shape);
  Tensor2D<float> output_tensor(layer_output_flattened, h, w);
  std::vector<std::vector<float> > final_boxes;
	final_boxes.clear();
	final_boxes = m_decoder->decode(output_tensor, m_decoder->score_threshold, m_decoder->nms_threshold);
    std::vector<unified_detector::BoundingBox> boxes = detector_utils::get_boxes_from_2D_tensors(final_boxes, image);
    result_boxes = boxes;
    if(!boxes.empty()){
        result_boxes = this->roi_mapper->map(result_boxes);
        result_boxes = this->pet_roi_filterer->filter(result_boxes, target_image,mode);
        result_boxes = this->text_roi_filterer->filter(result_boxes, target_image,mode);
        result_boxes = detector_utils::correct_roi_boxes(result_boxes, target_width,target_height,image_rotation,width,height);
      }else{
        result_boxes.clear();
        LOGV("No boxes found returning false");
        auto execute_finish = CURRENT_TIME;
        LOGV("VZ Debug: Execute from AAR: Total time taken: %lld ms",execute_finish - start_time);
        return false;
    }
    auto execute_finish = CURRENT_TIME;
    LOGV("VZ Debug: Execute from AAR: Total time taken: %lld ms",execute_finish - start_time);
    return true;
  }
  return false;

}
void UnifiedDetectorImpl::get_tracking_non_tracking_boxes(const std::vector<unified_detector::BoundingBox>& detector_boxes,
                                                          std::vector<unified_detector::BoundingBox>& tracker_boxes,
                                                          std::vector<unified_detector::BoundingBox>& non_track_boxes){
  tracker_boxes.clear();
  non_track_boxes.clear();
  tracker_boxes.reserve(detector_boxes.size());
  non_track_boxes.reserve(detector_boxes.size());
  for(size_t i=0; i<detector_boxes.size(); i++){
      if(detector_boxes[i].tag_id == PETFACE_ID || detector_boxes[i].tag_id == PET_ID){ //Pet Detector case Model class 8 -> PetFace , 11 -> PET
#ifdef DUMPBOX
        LOGV("VZ Debug Det Label execute_with_tracking:[%s], Det Coord:left:[%d],top:[%d],right:[%d],bottom:[%d], score:[%f]", \
        detector_boxes[i].tag.c_str(), detector_boxes[i].location.left, detector_boxes[i].location.top, detector_boxes[i].location.right, detector_boxes[i].location.bottom, detector_boxes[i].score);
#endif
        tracker_boxes.push_back(detector_boxes[i]);
      }else{
          non_track_boxes.push_back(detector_boxes[i]);
      }
  }
}
 
  bool UnifiedDetectorImpl::execute_with_tracking(char* image_buffer, int width, int height, int image_rotation, UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format, UNIFIED_DETECTOR_MODE::Mode mode, UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType zoom_type, char* file_name){
   std::lock_guard<std::mutex> guard(this->execute_tracking_mtx);
   LOGV("execute_with_tracking: image_rotation : [%d], width : [%d], height : [%d], image_format: [%d], mode : [%d]",image_rotation,width,height,image_format, static_cast<int>(mode));

   m_frame_number++;
   if (!this->init_done) {
       LOGV("VZ Debug: Execute called without proper init completion. Execute aborted !");
       return false;
   }
   else if(width <=0 || height <= 0){
     LOGV("Image meta data is not proper.. image width : %d, image height : %d",width,height);
     return false;
   }


   // Initialize video start time for the first frame
   if (m_frame_number == 1) {
       video_start_time = CURRENT_TIME;
       cumulative_tracking_time = 0;
   }
   auto start_time = CURRENT_TIME;
  bool execute_status = execute(image_buffer,width,height,image_rotation, image_format,false,mode,zoom_type,file_name);
  // m_frame_number++;
  if(execute_status){
     if(!normal_ocsort_tracker){
         normal_ocsort_tracker = new ocsort::OCSort(track_thresh, max_age, 1 /* min_hits */, iou_threshold, delta_t, "iou", 0.2, false);
     }
     if(!zoom_ocsort_tracker){
         zoom_ocsort_tracker = new ocsort::OCSort(track_thresh, max_age, 1 /* min_hits */, iou_threshold, delta_t, "iou", 0.2, false);
     }
    
     // Reuse storage between calls — no per-frame heap allocs for these vectors
     static thread_local std::vector<unified_detector::BoundingBox> trackboxes;
     static thread_local std::vector<unified_detector::BoundingBox> non_tracked_boxes;
     get_tracking_non_tracking_boxes(result_boxes, trackboxes, non_tracked_boxes);

     // Cache class-name strings — avoids 2 std::string heap allocs per frame
     static const std::string pets("pets");
     static const std::string petface("petface");

     // Convert trackboxes to Eigen::MatrixXf format for OCSort
     // OCSort::update expects 6 columns: [x1, y1, x2, y2, score, class_id]
     Eigen::MatrixXf dets(trackboxes.size(), 6);
     for (size_t i = 0; i < trackboxes.size(); ++i) {
         dets(i, 0) = trackboxes[i].location.left;
         dets(i, 1) = trackboxes[i].location.top;
         dets(i, 2) = trackboxes[i].location.right;
         dets(i, 3) = trackboxes[i].location.bottom;
         dets(i, 4) = trackboxes[i].score;
         dets(i, 5) = static_cast<float>(trackboxes[i].tag_id);
     }

     LOGV("VZ Debug: execute_with_tracking zoom_type=[%d], trackboxes.size=[%zu]",
          static_cast<int>(zoom_type), trackboxes.size());

     // Update OCSort trackers
     std::vector<Eigen::RowVectorXf> tracks;
     // Measure pure tracking time
     auto tracking_start_time = CURRENT_TIME;
     if(zoom_type == UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType::ZOOM){
         tracks = zoom_ocsort_tracker->update(dets);
     }else{
         tracks = normal_ocsort_tracker->update(dets);
     }
     auto tracking_end_time = CURRENT_TIME;
     long long tracking_time = tracking_end_time - tracking_start_time;
     cumulative_tracking_time += tracking_time;
     
     // Log tracking performance metrics
     LOGV("VZ Debug: Tracking time for frame %d: %lld ms", m_frame_number, tracking_time);
     LOGV("VZ Debug: Cumulative tracking time: %lld ms", cumulative_tracking_time);
     LOGV("VZ Debug: #tracks size execute : [%zu]" , tracks.size());

     // tracked_boxes is the same content as trackboxes (the duplicate from the old tuple is gone).
     // We use bool-mark-as-claimed instead of vector::erase to avoid O(N^2) erasures.
     const std::vector<unified_detector::BoundingBox>& tracked_boxes = trackboxes;
     static thread_local std::vector<bool> tracked_box_claimed;
     tracked_box_claimed.assign(tracked_boxes.size(), false);

     result_boxes.clear();
     movingAvg tempTrackedBox;
     for (const auto& track : tracks) {
         if (track.size() >= 5) {
             int track_id = static_cast<int>(track(4));
             float tx1 = track(0), ty1 = track(1), tx2 = track(2), ty2 = track(3);
             float score = 0.0f; // Default score

             // Find the best matching detection box for this track (skip already-claimed)
             unified_detector::BoundingBox detector_box;
             int best_idx = -1;
             float best_dist = std::numeric_limits<float>::max();
             for (size_t j = 0; j < tracked_boxes.size(); j++) {
                 if (tracked_box_claimed[j]) continue;
                 float dx1 = tx1 - tracked_boxes[j].location.left;
                 float dy1 = ty1 - tracked_boxes[j].location.top;
                 float dx2 = tx2 - tracked_boxes[j].location.right;
                 float dy2 = ty2 - tracked_boxes[j].location.bottom;
                 float dist = dx1*dx1 + dy1*dy1 + dx2*dx2 + dy2*dy2;
                 if (dist < best_dist) {
                     best_dist = dist;
                     best_idx = j;
                     score = tracked_boxes[j].score;
                 }
             }

             if (best_idx == -1) {
                 LOGV("VZ Debug: execute_with_tracking: track_id=[%d] has no matching detector box in this frame, skipping", track_id);
                 continue;
             }
             detector_box = tracked_boxes[best_idx];
             tracked_box_claimed[best_idx] = true; // O(1) mark — no vector erase
             
             unified_detector::Rectangle avgResultBox;
             int index = 0;
             if(movingAvgVec.size()){
                 auto low = std::lower_bound(movingAvgVec.begin(), movingAvgVec.end(), track_id, movingAvg::less_than());
                 if(low >= movingAvgVec.begin() && low < movingAvgVec.end()){
                     index = (low-movingAvgVec.begin());
                     #ifdef DUMPBOX
                         LOGV("VZ Debug: execute_with_tracking: index incase of track id present: [%d], trackid : [%d], movingAvgVec Size : [%d]",index, movingAvgVec[index].trackid, movingAvgVec.size());
                     #endif
                     unified_detector::Rectangle trackedBox = unified_detector::Rectangle((int)tx1, (int)ty1, (int)tx2, (int)ty2);
                     float iou = tracker_utils::get_overlap_between_boxes(movingAvgVec[index].smoothed_box, detector_box.location);
                     iou = (iou < 0.0f) ? -(iou): iou;
                     int interSectionArea = iou*100;
                     #ifdef DUMPBOX
                         LOGV("VZ Debug execute: interSectionArea : [%d], trackid : [%d]",interSectionArea, movingAvgVec[index].trackid);
                     #endif
                     if(interSectionArea < 80){
                         movingAvgVec[index].smoothed_box = detector_box.location;
                     }else{
                         movingAvgVec[index].smoothed_box = tracker_utils::update_smother_box(detector_box.location, movingAvgVec[index].smoothed_box);
                     }
                     avgResultBox = movingAvgVec[index].smoothed_box;
                 }else{
                     movingAvg tempMovingAvg;
                     auto low = movingAvgVec.end();
                     index = (low-movingAvgVec.begin());
                     tempMovingAvg.counter = 1;
                     tempMovingAvg.trackid = track_id;
                     movingAvgVec.emplace_back(tempMovingAvg);
                     #ifdef DUMPBOX
                         LOGV("VZ Debug: execute_with_tracking: index incase of track id not present: [%d], trackid : [%d], movingAvgVec Size : [%d]",index, movingAvgVec[index].trackid, movingAvgVec.size());
                     #endif
                     avgResultBox = detector_box.location;
                     movingAvgVec[index].smoothed_box = detector_box.location;
                 }
             }else{
                 movingAvg tempMovingAvg;
                 tempMovingAvg.counter = 1;
                 tempMovingAvg.trackid = track_id;
                 movingAvgVec.emplace_back(tempMovingAvg);
                 auto low = movingAvgVec.end();
                 index = (low-movingAvgVec.begin())-1;
                 #ifdef DUMPBOX
                     LOGV("VZ Debug: execute_with_tracking: index incase of vector empty: [%d], trackid : [%d], movingAvgVec Size : [%d]",index, movingAvgVec[index].trackid, movingAvgVec.size());
                 #endif
                 avgResultBox = detector_box.location;
                 movingAvgVec[index].smoothed_box = detector_box.location;
             }

             unified_detector::BoundingBox new_box({avgResultBox.left, avgResultBox.top, avgResultBox.right, avgResultBox.bottom},
             {width, height},
             (detector_box.tag_id == 8) ? petface : (detector_box.tag_id == 11) ? pets : std::string(""),
             score,
             detector_box.tag_id,
             track_id,
             false, true);
             result_boxes.emplace_back(new_box);
             if(m_frame_number==1){
                 prev_frame_pet_result_boxes.emplace_back(new_box);
             }
         }
     }
     if(result_boxes.size() == prev_frame_pet_result_boxes.size()){
       for(int i = 0;i<result_boxes.size();i++){
         if(result_boxes[i].trackid == prev_frame_pet_result_boxes[i].trackid){
           if(result_boxes[i].score < prev_frame_pet_result_boxes[i].score){
             result_boxes[i].score = prev_frame_pet_result_boxes[i].score;
           }
         }
       }
     }else{        //If two frames have different no of boxes(some pet left the frame), replace all prev frame boxes with current frame boxes
       prev_frame_pet_result_boxes.clear();
       for(int i = 0;i<result_boxes.size();i++){
          prev_frame_pet_result_boxes.emplace_back(result_boxes[i]);
       }
     }    
               
     for(int i =0; i<non_tracked_boxes.size();i++){
       unified_detector::BoundingBox new_box({non_tracked_boxes[i].location.left, non_tracked_boxes[i].location.top,
         non_tracked_boxes[i].location.right, non_tracked_boxes[i].location.bottom},
       {width, height},
       non_tracked_boxes[i].tag, non_tracked_boxes[i].score, non_tracked_boxes[i].tag_id,-1,false, true);
       result_boxes.emplace_back(new_box);
     }
     
     if(!result_boxes.empty())
         sort(result_boxes.begin(),result_boxes.end(), &confidenceComparator);
    }else{
      auto execute_finish = CURRENT_TIME;
        LOGV("VZ Debug: Execute with tracking: Total time taken: %lld ms",execute_finish - start_time);
        LOGV("Execute failed no tracking returning false")
        return false;
    }
    auto execute_finish = CURRENT_TIME;
        LOGV("VZ Debug: Execute with tracking: Total time taken: %lld ms",execute_finish - start_time);
        LOGV("VZ Debug: Video processing frame %d, cumulative tracking time: %lld ms", m_frame_number, cumulative_tracking_time);
     return true;
 }
 
 /**
  * @brief Interface to get the ROI boxes
  * @details extracts all the boxes post detection
  *
  * @param[in] none
  *
  * @return number of boxes
  */
 int  UnifiedDetectorImpl::get_n_boxes(){
     return result_boxes.size();
 }
 /**
 * @brief check the text is present on the scene or not
 *
 *
 * @return bool text is present or not
 */
 bool UnifiedDetectorImpl::contains_doc_text(){
    if(result_boxes.empty()){
        return false;
      }
      for(int i=0; i<result_boxes.size();i++){
        if(result_boxes[i].tag_id == static_cast<int>(unified_detector::UnifiedDetector::TagCategory::TAG_DOC_TEXT)){
          LOGV("VZ Debug: contains_doc_text is true");
          return true;
        }
      }
      return false;
 }

 int  UnifiedDetectorImpl::get_saliency_boxes_to_center_count(){
  result_boxes = detector_utils::get_saliency_boxes_to_center(result_boxes,m_width,m_height);
  return result_boxes.size();
}

 /**
  * @brief Interface to bet the ROI bixes
  * @details extracts all the boxes post detection
  *
  * @param[in] none
  *
  * @return number of boxes
  */
  int  UnifiedDetectorImpl::get_n_tags(int max_scene_infos){
    return result_boxes.size();
}
bool UnifiedDetectorImpl::is_range(int id){
  if(id>=100 && id<=105)
    return true;
  else
    return false;
}
/**
  * @brief Interface to return desired boxes
  * @details Sorts the boxes by category and returns the most confident box from each category first
  *
  * @param[in] result_boxes contains the final output boxes
  * @param[in] size of detector_boxess
  *
  * @return bool status
  */
std::vector<unified_detector::BoundingBox> UnifiedDetectorImpl::get_classwise_sorted_boxes_by_confidence(std::vector<unified_detector::BoundingBox>& result_boxes, int n){
  std::vector<unified_detector::BoundingBox> final_boxes;
  LOGV("VZ Debug: Sorting [%d] boxes", n);
  //LOGV("VZ Debug: Sorting logic V2");
  final_boxes.reserve(n);
  std::unordered_map<int,int> best_index;
  std::vector<int> class_order;

  for(int i=0;i<n;i++){
    int cat = result_boxes[i].tag_id;

    if(!best_index.count(cat)){
      best_index[cat] = i;
      class_order.push_back(cat);
    }else{
      if(result_boxes[i].score > result_boxes[best_index[cat]].score){
        best_index[cat] = i;
      }
    }
  }
  for(int cat: class_order){
    final_boxes.push_back(result_boxes[best_index[cat]]);
  }
  for(int i=0;i<n;i++){
    if(best_index[result_boxes[i].tag_id] == i)
        continue;
    final_boxes.push_back(result_boxes[i]);
  }
  LOGV("VZ Debug: Finished sorting [%zu] boxes", final_boxes.size());
  return final_boxes;
}
//VEX APIs
long UnifiedDetectorImpl::get_supported_mode_count(){
  return supported_modes.size(); //OD, ODCLS only supported.
}

int UnifiedDetectorImpl::get_supported_modes(int* modes){
  for(int i=0;i<supported_modes.size();i++){
    modes[i] = static_cast<int>(supported_modes[i]);
  }
  return 0;//STATUS_OK
}

long UnifiedDetectorImpl::get_supported_tag_count(int mode){
  if(mode == static_cast<int>(UNIFIED_DETECTOR_MODE::Mode::OD))
    return od_tags.size();
  else if(mode == static_cast<int>(UNIFIED_DETECTOR_MODE::Mode::ODCLS))
    return odcls_tags.size();
  else{
    LOGV("VZ Debug: Incorrect mode passed as argument! Please provide supported mode only.");
    return 0;
  }
}

int UnifiedDetectorImpl::get_supported_tags(int mode,const char** tags){
  vex_tags_str.clear();
  if(mode == static_cast<int>(UNIFIED_DETECTOR_MODE::Mode::OD)){
    for(auto tag: od_tags){
        //LOGV("Current tag is: [%s]", tag.second.c_str());
        vex_tags_str.push_back(tag.second);
    }
    for(int i=0;i<od_tags.size();i++){
        tags[i] = (char*)vex_tags_str[i].c_str();
    }
  }
  else if(mode == static_cast<int>(UNIFIED_DETECTOR_MODE::Mode::ODCLS)){
    for(auto tag: odcls_tags){
      //LOGV("Current tag is: [%s]", tag.second.c_str());
      vex_tags_str.push_back(tag.second);
    }
    for(int i=0;i<odcls_tags.size();i++){
      tags[i] = (char*)vex_tags_str[i].c_str();
    }
  }
  else{
    LOGV("VZ Debug: Incorrect mode passed as argument! Please provide supported mode only.")
    return 1; //STATUS_ERR
  }
  return 0;//STATUS_OK
}

 /**
  * @brief ROI box details
  * @details extracts all the boxes post detection
  *
  * @param[in] tag_infos contains the details of ROI
  * @param[in] size of detector_boxess
  *
  * @return bool status
  */
 bool UnifiedDetectorImpl::get_tag_info(TagInfo tag_infos[], int n){
     std::lock_guard<std::mutex> guard_extraction(this->result_extraction_mtx);
     if(result_boxes.empty()){
        LG<<"VZ Debug: No boxes found hence returning false";
        return false;
      }
      if(n > this->get_n_boxes()){
        n = this->get_n_boxes();
      }
      LOGV("VZ Debug: No. of boxes to populate tag_info: [%d]", n);
      std::vector<unified_detector::BoundingBox> final_boxes = get_classwise_sorted_boxes_by_confidence(result_boxes, result_boxes.size());
      //VEX TESTING
    #ifdef TESTING
      LOGV("VZ Debug: Before VEX");
       long supported_modes_count = get_supported_mode_count();
       int modes[supported_modes_count];
       int status_mode = get_supported_modes(modes);
       long supported_tag_count = get_supported_tag_count(2);
       const char **tags_vex = new const char*[supported_tag_count];
       int supported_tags_status = get_supported_tags(2,tags_vex);
       LOGV("VZ Debug: VEX supported_modes_count: [%lu]; supported_tag_count: [%lu]",supported_modes_count, supported_tag_count);
       for(int i=0;i<supported_modes_count;i++){
          LOGV("VZ Debug: VEX supported_modes: [%d]", modes[i]);
       }
       for(int j=0;j<supported_tag_count;j++){
         LOGV("VZ Debug: VEX supported_tags: [%s]", tags_vex[j]);
       }
       for (int i = 0; i < supported_tag_count; ++i) {
        if (tags_vex[i]) {  // Check if pointer is valid
          delete[] const_cast<char*>(tags_vex[i]);
        } 
      }
      delete[] tags_vex;
      LOGV("VZ Debug: After VEX");
    #endif //TESTING_VEX
      //END VEX TESTING
      #ifdef DUMPBOX
        LOGV("VZ Debug: get_tag_info num boxes: [%d]", n);
      #endif
      for(int i=0; i<n; i++){
        LG<<"VZ Debug: get_tag_info detected class_id: ["<<final_boxes[i].tag_id<<"], score: ["<<final_boxes[i].score<<"]";
        LOGV("VZ Debug: get_tag_info_debug detected trackid : [%d], class_id:[%d], score:[%f]",final_boxes[i].trackid, final_boxes[i].tag_id, final_boxes[i].score);
        tag_infos[i].trackid        = final_boxes[i].trackid;
        tag_infos[i].score          = final_boxes[i].score;

        tag_infos[i].left           = final_boxes[i].location.left;
        tag_infos[i].top            = final_boxes[i].location.top;
        tag_infos[i].right          = final_boxes[i].location.right;
        tag_infos[i].bottom         = final_boxes[i].location.bottom;
        tag_infos[i].image_width    = final_boxes[i].image_size.width;
        tag_infos[i].image_height   = final_boxes[i].image_size.height;
        int tag_key = final_boxes[i].tag_id;
        //LOGV("VZ Debug: get_tag_info tag_key: [%d]", tag_key);
        if(final_boxes[i].tag_id == 11){
          tag_infos[i].tag_class      = unified_detector::UnifiedDetector::TagCategory::TAG_PET; //Pet Detector case Camera Module  PET -> 10
          tag_infos[i].tag = (odcls_tags.find(static_cast<int>(tag_infos[i].tag_class)) == odcls_tags.end())?(char*)final_boxes[i].tag.c_str():(char*)odcls_tags[static_cast<int>(tag_infos[i].tag_class)].c_str();
        } else if(final_boxes[i].tag_id == 10){
          tag_infos[i].tag_class      = unified_detector::UnifiedDetector::TagCategory::TAG_HUMANHEAD; //Pet Detector case camera Module HUMAN HEAD -> 11 
          tag_infos[i].tag            = "human_head";
        }
        else if(final_boxes[i].tag_id == 14){
          tag_infos[i].tag_class = unified_detector::UnifiedDetector::TagCategory::TAG_WINE;
          tag_infos[i].tag = (odcls_tags.find(static_cast<int>(tag_infos[i].tag_class)) == odcls_tags.end())?(char*)final_boxes[i].tag.c_str():(char*)odcls_tags[static_cast<int>(tag_infos[i].tag_class)].c_str();
        }
        else if(final_boxes[i].tag_id == 15){
          tag_infos[i].tag_class = unified_detector::UnifiedDetector::TagCategory::TAG_WINELABEL;
          tag_infos[i].tag = (odcls_tags.find(static_cast<int>(tag_infos[i].tag_class)) == odcls_tags.end())?(char*)final_boxes[i].tag.c_str():(char*)odcls_tags[static_cast<int>(tag_infos[i].tag_class)].c_str();
        }
        else{
          tag_infos[i].tag_class      = (unified_detector::UnifiedDetector::TagCategory)(final_boxes[i].tag_id);
          //LOGV("VZ Debug: The resultant tag of final boxes is: [%s] and id is: [%d]", final_boxes[i].tag.c_str(), final_boxes[i].tag_id);
          //std::transform(final_boxes[i].tag.begin(), final_boxes[i].tag.end(), final_boxes[i].tag.begin(), ::tolower); //convert tag name to lowercase for vex compatibility
          //tag_infos[i].tag            = (char*) final_boxes[i].tag.c_str();
          tag_infos[i].tag = (odcls_tags.find(static_cast<int>(tag_infos[i].tag_class)) == odcls_tags.end())?(char*)final_boxes[i].tag.c_str():(char*)odcls_tags[static_cast<int>(tag_infos[i].tag_class)].c_str();
        }
        (tag_infos[i].trackid != -1)?tag_infos[i].supportedtrackid = true : tag_infos[i].supportedtrackid = false;

        tag_infos[i].isSceneCategory = final_boxes[i].isSceneCategory;
        tag_infos[i].isODCategory = final_boxes[i].isObjectCategory;

        if(is_range(final_boxes[i].tag_id)){
        tag_infos[i].isSceneCategory = true;
        tag_infos[i].isODCategory = false;
        }
        //std::string display_tag = odcls_tags[static_cast<int>(tag_infos[i].tag_class)];
        //#ifdef TESTING
        //LG<<"VZ Debug: get_tag_info detected trackid : ["<<tag_infos[i].trackid<<"],class_id: ["<<static_cast<int>(tag_infos[i].tag_class)<<"], class: ["<<display_tag<<"], score: ["<<tag_infos[i].score<<"], left : ["<<tag_infos[i].left<<"], top : ["<<tag_infos[i].top<<"], bottom : ["<<tag_infos[i].bottom<<"], right : ["<<tag_infos[i].right<<"], isSceneCategory: ["<<tag_infos[i].isSceneCategory<<"], isObjectCategory: ["<<tag_infos[i].isODCategory<<"]";
        //LG<<"VZ Debug: get_tag_info detected trackid : ["<<tag_infos[i].trackid<<"],class_id: ["<<static_cast<int>(tag_infos[i].tag_class)<<"], score: ["<<tag_infos[i].score<<"], left : ["<<tag_infos[i].left<<"], top : ["<<tag_infos[i].top<<"], bottom : ["<<tag_infos[i].bottom<<"], right : ["<<tag_infos[i].right<<"], isSceneCategory: ["<<tag_infos[i].isSceneCategory<<"], isObjectCategory: ["<<tag_infos[i].isODCategory<<"]";
        //LG<<"VZ Debug: get_tag_info detected class_id: ["<<static_cast<int>(tag_infos[i].tag_class)<<"], score: ["<<tag_infos[i].score<<"]";
        //#endif
      }
      return true;
 }

 