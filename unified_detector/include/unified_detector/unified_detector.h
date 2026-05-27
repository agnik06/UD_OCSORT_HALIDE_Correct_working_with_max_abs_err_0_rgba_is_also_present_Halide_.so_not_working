#ifndef __UINIFIED_DETECTOR__
#define __UINIFIED_DETECTOR__
namespace unified_detector{
class UnifiedDetector{
public:
    struct UNIFIED_DETECTOR_MODE {
        /**
         * Modes supported by Unified Detector
         */
        typedef enum {
            OD = 0,
            CLS = 1,
            ODCLS = 2,
            CUSTOM_NIGHT =3,
            CUSTOM_SINGLE_TAKE =4,
            CUSTOM_PET_STUDIO =5,
            CUSTOM_REMASTER =6,
            CUSTOM_ANY = 7
        }Mode;
        Mode mode;
    };
    //enum for status of execution(Vex)
    enum class UNIFIED_DETECTOR_STATUS{
        STATUS_OK = 0,
        STATUS_ERR = 1
    };
    struct UNIFIED_DETECTOR_EXECUTE_PARAM {
         /**
         * Image formats supported by Unified Detector
         */
        typedef enum {
            NV21 = 0,
            BGR  = 1,
            RGBA = 2,
            NV12 = 3
        }ImageFormat;
        /**
        * To know the the image buffer is zoomed or non zoomed buffer
        */
        typedef enum {
            NORMAL = 0,
            ZOOM  = 1,
            ANY_ZOOM =2
        }ZoomType;
        char *image_buffer;
        ImageFormat imageFormat;
        int width;  // Width of input image
        int height; // Height of input image
        int image_rotation; // Rotation angle of input image
        ZoomType zoomType;
        bool reset_and_track;
        //char* filename;
    };

    /**
     * Categories supported by detector and classifier
     */
     enum class TagCategory {
        TAG_INVALID = 0,
        TAG_OBJECT = 1,
        TAG_PERSON = 2,
        TAG_FOOD = 3,
        TAG_VEHICLE = 4,
        TAG_FLOWER = 5,
        TAG_ANIMAL = 6,
        TAG_DRINK =7,
        TAG_PETFACE = 8,
        TAG_MOON = 9,
        TAG_PET = 10,
        TAG_HUMANHEAD = 11, 
        TAG_TEXT = 12,
        TAG_SCENE_TEXT = 13,
        TAG_PET_EYES = 14,
        TAG_WINE = 15,
        TAG_WINELABEL = 16,      // This represents generic text. It is different from document text (TAG_DOC_TEXT).
        TAG_DOC_TEXT = 17,  // This represents document text. It is different from generic text (TAG_TEXT).
        TAG_LOD_CID18 = 18, // These are custom tags for LOD. Can be used by LOD only. Not available in other detectors. 18-25 are reserved for LOD. 26 onwards can be used by other detectors.
        TAG_LOD_CID19 = 19,
        TAG_LOD_CID20 = 20,
        TAG_LOD_CID21 = 21,
        TAG_LOD_CID22 = 22,
        TAG_LOD_CID23 = 23,
        TAG_LOD_CID24 = 24,
        TAG_LOD_CID25 = 25,
        //Classifier categories
        TAG_SUNSET_SUNRISE = 100,
        TAG_SNOW = 101,
        TAG_HOMEINDOOR = 102,
        TAG_SCENERY = 103,
        TAG_GREENERY = 104,
        TAG_RESTAURANT_INDOOR = 105,
        TAG_BABY = 106,               
        TAG_CAT = 107,                
        TAG_DOG = 108,                
        TAG_CLOTHING = 109,    	   
        TAG_TREE = 110,               
        TAG_PEOPLE = 111,             
        TAG_MOUNTAIN = 112,  
        TAG_MOUNTAIN_GREEN = 113,               
        TAG_MOUNTAIN_FALL = 114,	           
        TAG_BEACH = 115,           
        TAG_NIGHTVIEW = 116,         
        TAG_WATERFALL = 117,
        TAG_WATERSIDE = 118,
        TAG_STAGE = 119,
        TAG_TREE_GREEN = 120,
        TAG_SKY_BLUE = 121,
        TAG_SKY_GREY = 122,
        TAG_SKYSCRAPER = 123,
        TAG_CITY = 124,
        TAG_SHOE_DISP = 125,          
        TAG_SHOE_ON = 126
    };

    /**
     * Details of ROI from detector and classifier
     */
    struct TagInfo{
        TagCategory tag_class;
        char* tag;
        float score;

        int left;                   
        int top;
        int right;
        int bottom;                  
        int image_width;             //width and height of the original image in which the tag was detected
        int image_height;             
        int trackid;                 //track id for the tag, -1 if not supported for this tag
        
        bool supportedtrackid;       //if track id is supported for this tag or not
        bool isSceneCategory;        //if it is a scene category or not
        bool isODCategory;           //if it is a object category or not
    };

    /**
    * @brief Initializes Object detection with callback
    *
    * @param[in] Invokes Callback on succcessful initialization
    * @param[in] param contains the mode like OD,ODCLS etc
    * @param[in] Path of model in device
    *
    * @return bool status of initialization
    */
    bool initialize(void (*cb)(bool open_status),UNIFIED_DETECTOR_MODE mode, char* model_base_path);

    /**
     * @brief Initializes Object detection
     *
     * @param[in] param contains the mode like OD,ODCLS etc
     * @param[in] Path of model in device
     *
     * @return bool status of initialization
     */
    bool initialize(UNIFIED_DETECTOR_MODE mode, char* model_base_path);
 
    /**
     * @brief Performs inference on an image buffer
     *
     * @param[in] image_buffer defines the image buffer
     * @param[in] param contains the mode like OD,ODCLS etc, width, height image_rotation, image_format and zoomtype
     *
     * @return bool status of inference
     */
    bool execute(UNIFIED_DETECTOR_MODE mode, UNIFIED_DETECTOR_EXECUTE_PARAM param, char *file_name=0);

    /**
    * @brief Performs inference on an image buffer
    *
    * @param[in] image_buffer defines the image buffer
    * @param[in] param contains the mode like OD,ODCLS etc, width, height image_rotation, image_format and zoomtype
    *
    * @return bool status of inference
    */
    bool execute_with_tracking(UNIFIED_DETECTOR_MODE mode, UNIFIED_DETECTOR_EXECUTE_PARAM param,char *file_name=0);

    //This API for to support AAR/JAR
    bool execute(UNIFIED_DETECTOR_MODE mode, char* data, int width,int height,int image_rotation,unified_detector::UnifiedDetector::UNIFIED_DETECTOR_EXECUTE_PARAM::ImageFormat image_format,char *file_name=0);

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
     * @param[in] max_scene_infos defines the max number classifier scene infos to be returned
     *
     * @return number of ROI boxes and clasiifer tags
     */
     int  get_n_tags(int max_scene_infos);

    /**
     * @brief ROI box details
     *
     * @param[in] box_infos contains the details of ROI
     * @param[in] size of box_infos
     *
     * @return bool status
     */
    bool get_tag_info(TagInfo tag_infos[], int n);

    /**
     * @brief check the text is present on the scene or not
     *
     *
     * @return bool text is present or not
     */
     bool contains_doc_text();

     /**
     * @brief [Constuctor]Creates instance of Unified Detector
     *
     * @param[in] none
     *
     * @return none
     */
    static UnifiedDetector* Create();

    /**
     * @brief Destroy given instance Unified Detector
     *
     * @param[in] Unified Detector to delete
     *
     * @return none
     */
    static void Destroy(UnifiedDetector* unifieddetector);
    //Vex APIs
    long get_supported_mode_count();
    //0-STATUS_OK, 1-STATUS_ERR
    int get_supported_modes(int* modes);
    long get_supported_tag_count(int mode);
    //0-STATUS_OK, 1-STATUS_ERR
    int get_supported_tags(int mode, const char** tags);
};
} //namespace unified detector
#endif //__UNIFIED_DETECTOR__