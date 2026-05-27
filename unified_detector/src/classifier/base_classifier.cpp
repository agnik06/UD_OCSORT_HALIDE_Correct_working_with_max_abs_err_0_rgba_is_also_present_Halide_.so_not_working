#include "base_classifier.hpp"
#include<math.h>

BaseClassifier::BaseClassifier(std::shared_ptr<NNet> net_classifier)
    :net_classifier(net_classifier){

}
BaseClassifier::BaseClassifier(){

}
BaseClassifier::~BaseClassifier(){

}

std::vector<float> BaseClassifier::classify(cv::Mat image){
    int b, c, h, w;
    char* image_buffer = (char*)image.data;
    int width = image.cols;
    int height = image.rows;
    std::string classifier = "classifier";
    int result= net_classifier->forward_pass(image, classifier);
    #ifdef TESTING
      LOGV("Cached classifier from nnet from only textinvalid");
    #endif
    if(!result)
        return std::vector<float>();
    std::vector<float> image_features = get_softmax(net_classifier->classifier_op_name.c_str(), b, c, h, w);

    
    image_features = custom_softmax(image_features); //computes softmax for all classes except 833(text_invalid)

    

    return image_features;
}

std::vector<float> BaseClassifier::get_softmax(const char* layerName, int &b, int &c, int &h, int &w){
  float *layer_output_flattened = net_classifier->get_layer_output(layerName, b, c, h, w);
  if(!layer_output_flattened){
      LOGV("layer_output_flattened is empty");
      return std::vector<float>();
  }

  int op_size = b * c * h * w;
  //LOGV("Moon Debug: b[%d] * c[%d] * h[%d] * w[%d]", b, c, h, w);
  std::vector<float> softmax(layer_output_flattened, layer_output_flattened + op_size);
  /*#ifdef TESTING
      dump_outputnhwc_asraw(softmax,std::to_string(classifier_dump_number));
      classifier_dump_number++;
  #endif*/
#if TESTING
  LOGV("Moon Debug: b[%d] * c[%d] * h[%d] * w[%d]", b, c, h, w);
  char buffer[200] ={};
  for(int i =0; i< 10; i++)
  {
      char out_str[20] = {};
      sprintf(out_str,"[%3f] ", softmax[i]);
      //LOGV("Moon Debug: %x [%3f]", cur_output,  &cur_output);
      strcat(buffer, out_str);
  }
  LOGV("Moon Debug: %s", buffer);
#endif

  return softmax;
}

std::vector<float> BaseClassifier::custom_softmax(std::vector<float> feature)
{
	// to check DSP model
  if(feature.size()==0)
  {
    return feature;
  }
	float mx_val = 0.0;
	float sum = 0.0;
	mx_val = feature[0];
  int top1_raw_output_index = 0;
	for(int i =0;i<feature.size();i++)
	{
                if(i==833)
                {
                  continue;
                }
		mx_val = std::max(feature[i],mx_val);
    if(feature[i]>feature[top1_raw_output_index])
    {
      top1_raw_output_index = i;
    }
	}
  #ifdef TESTING
    LOGV("Top1 class before sigmoid output are %d, %f",top1_raw_output_index,feature[top1_raw_output_index]);
	#endif
  for(int i=0;i<feature.size();i++)
	{
               if(i==833)
                {
                  continue;
                }
		feature[i] = exp(feature[i]-mx_val);
		sum += feature[i];
	}
  int top1_class = 0;
  int top1_index = 0;
  if (std::abs(sum) > 1e-9) {
    for(int i=0;i<feature.size();i++)
    {
                  if(i==833)
                  {
                    continue;
                  }
      feature[i] = (feature[i]/sum);
      if(feature[i]>feature[top1_index])
      {
        top1_index = i;
      }
    }
  }
  #ifdef TESTING
    LOGV("Top1 class after sigmoid output are %d, %f",top1_index,feature[top1_index]);
  #endif
  #ifdef TESTING
	  LOGV("Applied sigmoid for artefact classes");
  #endif

	return feature;
}

