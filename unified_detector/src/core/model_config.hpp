/**
 * @file   model_config.hpp
 * @brief  This fine defines the type of the model Detecor or Classifier
 *
 * V1 refers to Mobiledetedgetpu series of models
 * V2 refers to MobileDetDSP series of models
 * Although V1 and V2 have same classes the thresholds will be
 * different since the NN arch is different across both
 */

 #ifndef __MODEL_COFIG__
 #define __MODEL_COFIG__
 
 namespace model{
     enum class type{
         DETECTOR = 0,
         CLASSIFIER = 1
     };
     enum class Version{
        UNKNOWN = 0,
        V1 = 1,
        V2 = 2
    };
 }
 
 #endif //__MODEL_COFIG__