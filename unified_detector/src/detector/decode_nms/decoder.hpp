#ifndef DECODER_GHOSTDET_H
#define Decoder_H

#include "core/framework/tensor.hpp"
#include "core/model_config.hpp"
#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

struct CenterPrior {
  int x;
  int y;
  int stride;
};

typedef struct DecodeBoxInfo {
  float x1;
  float y1;
  float x2;
  float y2;
  float score;
  int label;
} DecodeBoxInfo;

class Decoder {
public:
  Decoder();
  Decoder(int _target_width,int _target_height, model::Version version);

  ~Decoder();

  // static Decoder* decoder;
  // modify these parameters to the same with your config if you want to use
  // your own model
  int target_width=384, target_height=384;
  int input_size[2] = {target_width, target_height}; // input height and width
  
  int reg_max = 7; // `reg_max` set in the training config. Default: 7.
  const float score_threshold = 0.2;
	const float nms_threshold = 0.2;
  std::vector<int> strides = {8, 16, 32, 64}; // strides of the multi-level feature.
  std::vector<std::vector<float> > decode(Tensor2D<float> &feats, float score_threshold,
                              float nms_threshold);
  void set_target_dimen(int _target_width,int _target_height);
  std::vector<std::string> v1_labels{"Background", "Object", "Person",   "Food",
                                  "Vehicle",    "Flower", "Animal",   "Drink",
                                  "PetFace",    "Moon",   "HumanHead", "pets", "text", "sceneText", "wine", "wine_label"};
  std::vector<std::string> v2_labels{"Background", "Object", "Person",   "Food",
                                  "Vehicle",    "Flower", "Animal",   "Drink",
                                  "PetFace",    "Moon",   "HumanHead", "pets", "text", "sceneText", "wine", "wine_label"};
  std::vector<std::string> labels;
  int num_class;          // number of classes. 80 for COCO

private:
  void decode_infer(Tensor2D<float> &feats,
                    std::vector<CenterPrior> &center_priors, float threshold,
                    std::vector<std::vector<DecodeBoxInfo>> &results);
  DecodeBoxInfo disPred2Bbox(const float *dfl_det, int label, float score, int x,
                       int y, int stride);
  void nms(std::vector<DecodeBoxInfo> &result, float nms_threshold);
  std::vector<CenterPrior> center_priors;
  

};

#endif // GhostDet_H