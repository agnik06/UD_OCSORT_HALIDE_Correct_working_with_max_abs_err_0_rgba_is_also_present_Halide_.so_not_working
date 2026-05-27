#ifndef _DL_INTERFACE_HPP_
#define _DL_INTERFACE_HPP_

#include <memory>
#include <string>
#include <vector>

#ifndef SNAP_INTERFACE
#define SNAP_INTERFACE
typedef struct{
	typedef enum {
		SNAP_CPU = 0,
		SNAP_GPU = 1,
		SNAP_DSP = 2,
		SNAP_NPU = 3,
		SNAP_UNIT_MAXENUM = 4,
	}SNAPCompUnit;
	typedef enum {
		SNAP_CAFFE = 0,
		SNAP_SNPE = 1,
		SNAP_EDEN = 2,
		SNAP_TENSORFLOW = 3,
		SNAP_TENSORFLOWLITE = 4,
		SNAP_OFI = 5,
		SNAP_SNF = 6,
    SNAP_HVXNN = 7,
    SNAP_ONNX = 8,
    SNAP_SNAPLITE = 9,
    SNAP_TVM = 10,
    SNAP_MNP = 11,
		SNAP_MODEL_MAXENUM = 12,
	}SNAPModelType;
	typedef enum {
		SNAP_FLOAT32,
		SNAP_FLOAT16,
		SNAP_QASYMM16,
		SNAP_QASYMM8,
    SNAP_BFLOAT16,
		SNAP_DATATYPE_MAXENUM,
	}SNAPExecType;
	typedef enum {
		SNAP_HIGH_RESOLUTION,
    SNAP_FAST_INFERENCE,
    SNAP_BALANCED_INFERENCE,
		SNAP_OPTIONS_MAXENUM,
	}SNAPAddOptions;
	typedef enum {
		SNAP_DEBUG,
		SNAP_RELEASE,
		SNAP_MODE_MAXENUM,
	}SNAPMode;
	typedef enum {
		SNAP_SUSTAINED_PERFORMANCE,
		SNAP_HIGH_PERFORMANCE,
		SNAP_LOW_PERFORMANCE,
		SNAP_DVFS_MAXENUM,
	}SNAPDvfs;
  typedef enum {
    SNAP_DEFAULT,
    SNAP_CPU_XTENSIVE,
    SNAP_CPU_COMPUTE,
    SNAP_GPU_COMPUTE_CACHE,
    SNAP_GPU_COMPUTE_XTENSIVE_CACHE,
    SNAP_GPU_CL_CACHE,
    SNAP_SNAPLITE_CATALYST_MAXENUM,
  }SNAPSnapliteCatalyst;
	SNAPCompUnit compUnit = SNAP_UNIT_MAXENUM;
	SNAPExecType execType = SNAP_DATATYPE_MAXENUM;
	SNAPModelType mType = SNAP_MODEL_MAXENUM;
	std::vector<std::string> inputNames = {};
	std::vector<std::string> outputNames = {};        
	std::vector<float> scale = {};
	std::vector<float> mean = {};
	std::vector<float> stddev = {};
    std::string model_file = "";
	std::string weights_file = "";
	int cpuThreadCount = 0;
	bool allowReshape = false;
	std::vector<uint8_t> model_buffer = {};
	bool enable_secure = false;
	char *gpuCacheBuffer = 0;
	SNAPAddOptions flag = SNAP_OPTIONS_MAXENUM;
	std::string model_name = "";
	std::vector<std::vector<int32_t>> input_shape = {};
	SNAPMode mode = SNAP_MODE_MAXENUM;
  SNAPSnapliteCatalyst snaplite_catalyst = SNAP_DEFAULT;
	SNAPDvfs dvfs_perf = SNAP_DVFS_MAXENUM;
	size_t cacheSize = 0;
	uint8_t *model_buffer_ptr = 0; /* This is for optimized implementation. */
	size_t model_buffer_len = 0;
  int model_buffer_id = -1;	
}SNAPInterface;
#endif

enum class DLInterfaceImageFormat { BGR = 1 };

class DLInterface final {
 public:
  DLInterface();
  ~DLInterface();
  bool openDLModel(const char* filePath, const char* moduleName,
                   const std::vector<std::string>& output_layer_names,
                   int compute_unit_preference, int execution_data_type_preference);
  bool openDLModel(const char* filePath, const std::vector<std::string>& output_layer_names, 
                int compute_unit_preference, int execution_data_type_preference);                   
  bool getInputDimens(int& width, int& height);
  bool getInputChannels(int& channels);
  bool setChannelMeans(const std::vector<float>& channel_means);
  bool execute(float* imageData, int width, int height,
               DLInterfaceImageFormat imageFormat);

  float* getLayerOutput(const char* layerName, int& b, int& c, int& h, int& w);
  bool getLayerInputInfo(const char* layerName, int& c, int& h, int& w);
  class DLInterfaceImpl;

 private:
  DLInterface(const DLInterface&) = delete;
  DLInterface& operator=(const DLInterface&) = delete;
  std::unique_ptr<DLInterfaceImpl> pimpl;
};

#endif
