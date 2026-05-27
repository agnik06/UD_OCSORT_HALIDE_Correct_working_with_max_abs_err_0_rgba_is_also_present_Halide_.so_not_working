/**
 * @file   unified_detector_impl.hpp
 * @brief  This file implements the UnifiedDetector Interface
 *
 * This file implements UnifiedDetector Interface to be used by end api's
 */

 #ifndef __UNIFIED_DETECTORR_IMPL__
 #define __UNIFIED_DETECTORR_IMPL__
 #include "unified_detector.h" 
 #include <ctime>
 #include <string>
 #include <mutex>
 #include <memory>
 #include <net/NNet.hpp>
 #include "detector/detector_utils.hpp"
 #include "detector/tracker_utils.hpp"

 #include "detector/mapper/data/roi_data_handler.hpp"
 #include "detector/mapper/default_roi_mapper.hpp"
 #include "detector/mapper/data/roi_data_factory.hpp"
 #include "detector/filterer/roi_filterer.hpp"
 #include "detector/filterer/base_roi_filterer.hpp"
 #include "detector/filterer/pet_roi_filterer.hpp"
 #include "detector/filterer/text_roi_filterer.hpp"
 #include "detector/decode_nms/decoder.hpp"
 #include "detector/tracker/include/OCSort.hpp"
 #include "classifier/classifier_utils.hpp"
 #include "classifier/base_classifier.hpp"
 #include "classifier/decorator/classifier_with_flip.hpp"
 #include "classifier/decorator/classifier_with_verified_top_crop.hpp"
 #include "classifier/data_handler/data_handler.hpp"
 #include "classifier/data_handler/data/data_factory.hpp"
 #include "classifier/mapper/default_mapper.hpp"
 #include "classifier/filterer/base_classifier_filterer.hpp"
 #include "utils/image_utils.hpp"
 #include "utils/image_utils_neon.hpp" 
 #include "framework/entity.hpp"
 #include "snap/snap_wrapper.h"
 #include "snap/snap_utils.hpp"
 #include "../third_party/Eigen/Dense"

 typedef std::function<void(bool, std::string)> ImplCallback;
 class UnifiedDetectorImpl : public unified_detector::UnifiedDetector{
 
     enum Version{
         UNKNOWN = 0,
         V1 = 1,
         V2 = 2
     }; 
 public:
     UnifiedDetectorImpl();
     ~UnifiedDetectorImpl();
     UnifiedDetectorImpl(const UnifiedDetectorImpl&) = delete;
     UnifiedDetectorImpl& operator = (const UnifiedDetectorImpl&) = delete;
     /**
      * @brief Initializes Unified detection with callback
      *
      * @param[in] Invokes Callback on succcessful initialization
      * @param[in] Mode of operation
      * @param[in] Path of model in device
      *
      * @return bool status of initialization
      */
     bool initialize(void (*cb)(bool status),UNIFIED_DETECTOR_MODE::Mode mode, char* model_base_path);
     /**
      * @brief Initializes Unified detection
      *
      * @param[in] Mode of operation
      * @param[in] Path of model in device
      *
      * @return bool status of initialization
      */
     bool initialize(UNIFIED_DETECTOR_MODE::Mode mode, char* model_base_path);
     /**
      * @brief Performs inference on an image buffer
      *
      * @param[in] image_buffer defines the image buffer
      * @param[in] width of image
      * @param[in] height of image   
      * @param[in] image_rotation defines rotation angle of image
      * @param[in] image_format of given image(one of types in ImageFormat)
      *
      * @return bool status of inference
      */
     bool execute(char* image_buffer, int width, int height, int image_rotation, UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format, bool use_similarity,UNIFIED_DETECTOR_MODE::Mode mode, UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType zoom_type, char* file_name=0);
 
     /**
      * @brief check the text is present on the scene or not
      *
      *
      * @return bool text is present or not
      */
     bool contains_doc_text();
     int get_saliency_boxes_to_center_count();
     /**
      * @brief Number of boxes on inference
      *
      * @param[in] none
      *
      * @return number of ROI boxes
      */
     int  get_n_boxes();
     /**
      * @brief Number of boxes on inference
      *
      * @param[in] none
      *
      * @return number of ROI boxes
      */
      int  get_n_tags(int max_scene_infos);
    /**
     * @brief Range of tag ID
     * @details Checks if tag_id belongs to scene or object class
     *
     * @param[in] tag_id contains the tag_id
     *
     * @return bool status
     */
     bool is_range(int tag_id);
     /**
     * @brief Interface to return desired boxes
     * @details Sorts the boxes by category and returns the most confident box from each category first
     *
     * @param[in] result_boxes contains the final output boxes
     * @param[in] size of detector_boxess
     *
     * @return bool status
     */
    std::vector<unified_detector::BoundingBox> get_classwise_sorted_boxes_by_confidence(std::vector<unified_detector::BoundingBox>& result_boxes, int n);
     /**
      * @brief ROI box details
      *
      * @param[in] box_infos contains the details of ROI
      * @param[in] size of box_infos
      *
      * @return bool status
      */
     bool get_tag_info(TagInfo tag_infos[], int n);
     void (*init_cb)(bool);
     
     //Tracking interfaces
 
     bool execute_with_tracking(char* image_buffer, int width, int height, int image_rotation, UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,UNIFIED_DETECTOR_MODE::Mode mode, UNIFIED_DETECTOR_EXECUTE_PARAM::ZoomType zoom_type, char* file_name=0);
     void reset_ocsort_tracker();
     void initialize_callback();
     bool detector_initialize(UNIFIED_DETECTOR_MODE::Mode mode, std::string model_base_path);
     bool classifier_initialize(UNIFIED_DETECTOR_MODE::Mode mode, std::string model_base_path);
     bool execute(UNIFIED_DETECTOR_MODE::Mode mode, char* data, int width,int height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,char *file_name=0);

     //Vex APIs
     long get_supported_mode_count();
     int get_supported_modes(int* modes);
     long get_supported_tag_count(int mode);
     int get_supported_tags(int mode, const char** tags);
 
 private:

    struct movingAvg{
        int counter =0;
        int trackid = -1;
        unified_detector::Rectangle smoothed_box = unified_detector::Rectangle();
        struct less_than{
            bool operator()(const movingAvg &a, int id){
            return a.trackid < id;
            }
        };
    };
    int uodMovingAverageInterval = 4;
    std::vector<movingAvg> movingAvgVec;
     int m_imgWidth;
     int m_imgheight;
     int m_rotation;
     int m_orientation;
     UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat m_image_format;

     std::shared_ptr<NNet> detector_net = 0;
     std::shared_ptr<NNet> classifier_net =0;
     
     const int PETFACE_ID = 8;
     const int FLOWER_ID = 5;
     const int PET_ID = 11;
     const int TEXT_ID = 12;
     const int SCENETEXT_ID = 13;
     const int PERSON_ID = 2;
     const int FOOD_ID = 3;
 
     char* m_image_buffer;
     int m_width, m_height;
     bool detector_init_done;
     bool classifier_init_done;
     bool init_done;
     ImplCallback impl_init_cb;
     int m_frame_number;
     // Tracking performance metrics
     long long cumulative_tracking_time = 0;
     long long video_start_time = 0;

     SRIBSnapWrapper *classifier_snap_wrapper = NULL;
     SRIBSnapWrapper *detector_snap_wrapper = NULL;
     int detector_compute_unit;
     int classifier_compute_unit;
     std::string detector_model_name="";
     std::string classifier_model_name="";
 
     unified_detector::UnifiedDetector::UNIFIED_DETECTOR_MODE::Mode m_mode;
     std::string model_base_path="/vendor/etc/saiv/image_understanding/db/unified_detector/model.info";
 
    std::mutex initialize_mtx;
     std::mutex execute_mtx;
     std::mutex execute_tracking_mtx;
     std::mutex result_extraction_mtx;
     std::vector<std::string> detector_output_layer_names;
     std::vector<std::string> classifier_output_layer_names;
     std::vector<unified_detector::BoundingBox> result_boxes;
     std::vector<unified_detector::BoundingBox> prev_frame_pet_result_boxes;
     std::shared_ptr<IROIMapper> roi_mapper;
     std::shared_ptr<BaseROIFilterer> pet_roi_filterer;
     std::shared_ptr<BaseROIFilterer> text_roi_filterer;
     std::shared_ptr<Decoder> m_decoder;
     ocsort::OCSort* normal_ocsort_tracker;
     ocsort::OCSort* zoom_ocsort_tracker;
     const int frame_rate = 30; const int track_buffer = 20;
     const float track_thresh = 0.5; const float high_thresh = 0.6;
     const float match_thresh = 0.8;
     // OCSort parameters
     const int max_age = 30;
     const int min_hits = 3;
     const float iou_threshold = 0.3;
     const int delta_t = 3;

     std::shared_ptr<BaseClassifier> classifier;
     std::shared_ptr<ITagMapper> image_tag_mapper;
     std::shared_ptr<BaseClassifierFilterer> image_tag_filterer;
     //char* model_base_path = "/vendor/etc/saiv/image_understanding/db/unified_detector/model.info";

     std::vector<int> base_ids;

    #ifdef DUMPBOX
     int original_image_cnt = 0;
     int preprocess_image_cnt=0;
    #endif
     
     bool detector_execute(char* image_buffer, int width, int height, int image_rotation, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,UNIFIED_DETECTOR_MODE::Mode mode, bool& need_to_run_classifier, char* file_name=0);
     bool classifier_execute(char* image_buffer, int width, int height, int image_rotation, unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format, UNIFIED_DETECTOR_MODE::Mode mode, char* file_name=0);
     // Pass input by const ref + output vectors by reference to avoid tuple/vector copies in hot path.
     // Drops the redundant duplicate that the previous tuple-version returned.
     void get_tracking_non_tracking_boxes(const std::vector<unified_detector::BoundingBox>& detector_boxes,
                                          std::vector<unified_detector::BoundingBox>& tracker_boxes,
                                          std::vector<unified_detector::BoundingBox>& non_track_boxes);

     //Vex changes
     std::vector<std::string> vex_tags_str;
     std::vector<UNIFIED_DETECTOR_MODE::Mode> supported_modes = {
        UNIFIED_DETECTOR_MODE::Mode::OD,
        UNIFIED_DETECTOR_MODE::Mode::ODCLS
     };
     std::map<int, std::string> od_tags = {
        {2, "person"},
        {3, "food"},
        {5, "flower"},
        {7, "drink"},
        {8, "pet_face"},
        {9, "moon"},
        {10, "pet"},
        {12, "text"},
        {13, "scene_text"},
        {15, "wine"},
        {16, "wine_label"},
        {17, "document_text"}
     };
     std::map<int, std::string> odcls_tags ={
        {2, "person"},
        {3, "food"},
        {5, "flower"},
        {7, "drink"},
        {8, "pet_face"},
        {9, "moon"},
        {10, "pet"},
        {12, "text"},
        {13, "scene_text"},
        {15, "wine"},
        {16, "wine_label"},
        {17, "document_text"},
        {100, "sunrise_sunset"},
        {101, "snow"},
        {102, "home_indoor"},
        {103, "scenery"},
        {104, "greenery"},
        {105, "restaurant_indoor"}
     };
 };
 
 #endif //__UNIFIED_DETECTORR_IMPL__
 