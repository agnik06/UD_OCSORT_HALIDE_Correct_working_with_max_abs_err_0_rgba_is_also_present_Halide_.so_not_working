#ifndef SNAP_API_INCLUDE_SNAP_INTERFACE_H_
#define SNAP_API_INCLUDE_SNAP_INTERFACE_H_

#include <string>
#include <vector>

namespace snap {

/**
 *       @brief Describes possible status codes that will be returned by the
 * APIs
 */
enum ErrCode {
  OK = 0,
  ERR = 1,
  UNSUPPORTED_DEVICE = 2,
  /*Revisit: Add more error codes*/
};

/**
 *	@brief Describes the SnapVersions supported
 */
enum class SnapVersion {
  V0,
  /*Added buffer implementation*/
  V1,
  MAXENUM,
};

/**
 *       @brief Additional flags for High resolution Models
 */
enum class AddOptions {
  HIGH_RESOLUTION,
  FAST_INFERENCE,     /*fast_math*/
  BALANCED_INFERENCE, /*buffer backend*/
  MAXENUM,
};

/**
 *       @brief Verbose Or Release mode setting
 */
enum class Mode {
  DEBUG,
  RELEASE,
  MAXENUM,
};

/**
 *       @brief Dynamic Voltage and Frquency Scaling - performance modes
 */
enum class Dvfs {
  SUSTAINED_PERFORMANCE,
  HIGH_PERFORMANCE,
  LOW_PERFORMANCE,
  MAXENUM,
};

/**
 *	@brief Describes the type of data present in snap::DataBuffer
 */
enum class DataBufferType {
  FLOAT32 = 0,
  /** Android hardware Buffer type, this option is to be set by hidl servic
   * only, it is selected based on device capabilities */
  FLOAT32_AHWB = 1,
  /** Android Nativewindow Buffer type, this option is to be set by hidl service
   * only, it is selected based on device capabilities */
  FLOAT32_ANB = 2,
  MAXENUM = 1 << 30,
};

/**
 *	@brief Describes the Exectution DataType for SnapSession
 */
enum class ExecutionDataType {
  FLOAT32,
  FLOAT16,
  QASYMM16,
  QASYMM8,
  /** Special float16 option: length of mantissa and exponent are diff from IEEE
     standard*/
  BFLOAT16,
  MAXENUM,
};

/**
 *	@brief Describes one of the ComputingUnit to execute the model on
 */
enum class ComputeUnit {
  CPU,
  GPU,
  DSP,
  NPU,
  MAXENUM,
};

/**
 *	@brief Describes the type of model passed to the snapSession
 */
enum class ModelType {
  CAFFE,
  SNPE,
  EDEN,
  TENSORFLOW,
  TENSORFLOWLITE,
  OFI,
  SNF,
  HVXNN,
  ONNX,
  SNAPLITE,
  TVM,
  MNP,
  MAXENUM,
};

/**
 *	@brief Describes the format of input image data present in
 *       snap::DataBuffer
 */
enum class DataFormat {
  NCHW,
  NHWC,
  MAXENUM,
};

/**
 *	@brief Various buffer options for getting version of model
 *       type or GPU cache versions
 *       snap::BufferOption
 */
enum class BufferOption {
  /** Cache Path given as string in getBufferVersion */
  GPUCachePath = 0,
  /** Cache passed as buffer in getBufferVersion */
  GPUCacheBuffer = 1 << 0,
  /** Cache FD Passed to getBufferVersion */
  GPUCacheFD = 1 << 1,
  /** Model Path given as string in getBufferVersion */
  ModelPath = 1 << 2,
  /** Model passed as buffer in getBufferVersion */
  ModelBuffer = 1 << 3,
  /** Model FD Passed to getBufferVersion */
  ModelFD = 1 << 4,
  /** End of enum BufferOption */
  MAXENUM = 1 << 5,
};

/**
 * 	@brief Mode of SNPE Buffer execution
 */
enum class SnpeBufferType {
  /** default for execution on UserBuffer with TF8 encoding*/
  UB_TF8,
  /** execution on UserBuffer with float mode*/
  UB_FLOAT,
  /** execution on Itensor with float mode*/
  ITENSOR,
  /** end of enum SnpeBufferType */
  MAXENUM,
};

/**
 * 	@brief Options for execution on SnapLite
 */
enum class SnapliteCatalyst {
  /** default for CPU - no catalyst / GPU - opencl*/
  DEFAULT,
  /** standard cpu catalyst - snap cpu*/
  CPU_XTENSIVE,
  /** compute cpu catalyst - tbd */
  CPU_COMPUTE,
  /** compute catalyst with normal tuning */
  GPU_COMPUTE_CACHE,
  /** compute catalyst with extensive tuning */
  GPU_COMPUTE_XTENSIVE_CACHE,
  /** opencl catalyst with caching and tuning */
  GPU_CL_CACHE,
  /** end of enum SnapLiteCatalyst */
  MAXENUM,
};

/**
 * 	@brief Option for the mode of Gpu caching
 */
enum class GpuCacheOption {
  /** gpu Cache Option is not applicable */
  NOT_APPLICABLE,
  /** used when Local Gpu Cache is prefered */
  LOCAL,
  /** used when Global Gpu Cache is prefered */
  GLOBAL,
  /** end of enum GpuCacheOption */
  MAXENUM,
};

/**
 * 	@brief Cache structure handling serialized cache and its size
 */
struct SerializedGpuCacheBinary {
  SerializedGpuCacheBinary() : cache_ptr(nullptr), size(0) {}
  char *cache_ptr;
  size_t size;
};

/**
 * 	@brief structure for User Defined Layer (UDO)
 */
struct UserDefinedLayer {
  UserDefinedLayer()
      : path(""), name(""), execType(ExecutionDataType::MAXENUM),
        compUnit(ComputeUnit::MAXENUM) {}
  std::string path;
  std::string name;
  ExecutionDataType execType;
  ComputeUnit compUnit;
};

/**
 * @brief Options specific to Eden backend
 */
struct EdenConfiguration {
  EdenConfiguration()
      : eden_latency(0), eden_scheduling_priority(5), eden_npu_bound_core(0) {}
  /* latency is used as a hint which is transfered to the NPU governor to reduce
   * the power consumption. NPU governor dynamically lowers the operating
   * frequency of NPU,DNC,MIF,INT as much as possible while
   * trying to guarantee the latency. Unit is micro-seconds ex: for 15msec
   * latency = 15000**/
  uint32_t eden_latency;
  /* Higher the value, lower the scheduling priority.
   * Acceptable Range[0,10] with default value of 5 **/
  uint32_t eden_scheduling_priority;
  /* specifies which core to execute the DNN model on.
   * value 0 represents the first NPU core **/
  uint32_t eden_npu_bound_core;
};

/**
 * @brief SNAP Security perspective
 */
enum class SecurityLevel {
  /** No security */
  NO_SECURITY,
  /** Default AES based SNAP Security */
  SNAP_SECURITY,
  /** Samsung Knox Grade security for SNAP Models */
  KNOX_SECURITY,
  /** end of enum SnpeBufferType */
  MAXENUM,
};

/**
 * @brief  SNAP runtime profiler options
 */
enum class DebugLevel {
  /** No Logging */
  LEVEL0,
  /** number of layers (without affecting, performance) */
  LEVEL1,
  /** level1 + layerwise performance + memory [ This will degrade performance ]
   */
  LEVEL2,
  /** level1 + level2 + dot graph dumping */
  LEVEL3,
  MAXENUM,
};

/**
 * Options for SNAP-TVM-QNN backends to support cached model
 */
 struct SnapTvmQnnCacheOption {
  SnapTvmQnnCacheOption()
      : enableQnnCacheFmt(0) {}

  /**
   * set this flag to true for inferencing QNN cached model format
   */
  bool enableQnnCacheFmt;    
};

/**
*  Options specific SNAP Quantisation and Dequantisation Info
*/
struct QuantDequantInfo {
  QuantDequantInfo()
      : idx(0),
        scale(0),
        offset(0),
        minV(0),
        maxV(0) {}

  /**
   * index/encoding of the input or output layer
   */
  int idx;
  /**
   * represents the scaling factor
   */
  float scale;
  /**
   * represents the offset
   */
  int offset;
  /**
   * represents the minimum possible value
   */
  float minV;
  /**
   * represents the maximum possible value
   */
  float maxV;
};

/**
* Only valid for system based aidl client i.e. libsnap_aidl.snap.samsung.so
* Allows usercases to take control of input and output buffers
* Usecase need to specify the optimization mode in open before using shared mem apis
* Below modes will optimize execution by removing memcpy operation for in-out buffers
*/
enum class ExecuteOptimizeMode {
  /**
   * Input/Output Shared mem management done by snap aidl service
   */
  DEFAULT,
  /**
   * Input buffer shared mem management done by Usecase. Output done via service
   */
  INOPT,
  /**
   * Output buffer shared mem management done by Usecase. Input done via service
   */
  OUTOPT,
  /**
   * Input/Output Shared mem management done by Usecase
   */
  INOUTOPT,
  MAXENUM,
};

/**
*	@brief Describes the Performance profile Level
*/
enum class SnpePerfProfileLevel {
  LOW_BALANCED = 0,
  BALANCED = 1,
  HIGH_PERFORMANCE = 2,
  LOW_POWER_SAVER = 3,
  POWER_SAVER = 4,         
  HIGH_POWER_SAVER = 5,    
  SUSTAINED_HIGH_PERFORMANCE = 6,  
  SYSTEM_SETTINGS = 7,     
  BURST = 8,    
  EXTREME_POWER_SAVER = 9, 
  MAXENUM,
};
enum class TvmPerfProfileLevel {
  LOW_BALANCED = 0,
  BALANCED = 1,
  HIGH_PERFORMANCE = 2,
  LOW_POWER_SAVER = 3,
  POWER_SAVER = 4,         
  HIGH_POWER_SAVER = 5,    
  SUSTAINED_HIGH_PERFORMANCE = 6,  
  SYSTEM_SETTINGS = 7,     
  BURST = 8,    
  EXTREME_POWER_SAVER = 9, 
  MAXENUM,
};
enum class ComputePerfProfileLevel {
  LOW_BALANCED = 0,
  BALANCED = 1,
  HIGH_PERFORMANCE = 2,
  LOW_POWER_SAVER = 3,
  POWER_SAVER = 4,         
  HIGH_POWER_SAVER = 5,    
  SUSTAINED_HIGH_PERFORMANCE = 6,  
  SYSTEM_SETTINGS = 7,     
  BURST = 8,    
  MAXENUM,
};
enum class EnnPerfProfileLevel {
  MODE_LOW_POWER = 0,
  MODE_BOOST = 1,
  MODE_BALANCED = 2,
  MODE_CUSTOM = 3,
  MODE_PERFORMANCE = 4,
  MAXENUM,
};
/**
*	@brief Describes the Power profile
*/
struct PerfProfileLevel {
  PerfProfileLevel(){}
  
  union PerfProfileOpen {
      PerfProfileOpen()
          : snpeInitPerfSetting(SnpePerfProfileLevel::BURST){}
   
      SnpePerfProfileLevel snpeInitPerfSetting;
      TvmPerfProfileLevel tvmInitPerfSetting;
      ComputePerfProfileLevel computeInitPerfSetting;
      EnnPerfProfileLevel ennInitPerfSetting;
  }PerfProfileOpen;

union PerfProfileExec {
      PerfProfileExec()
          : snpeExecutePerfSetting(SnpePerfProfileLevel::SUSTAINED_HIGH_PERFORMANCE){}
    
      SnpePerfProfileLevel snpeExecutePerfSetting;
      TvmPerfProfileLevel tvmExecutePerfSetting;
      ComputePerfProfileLevel computeExecutePerfSetting;
      EnnPerfProfileLevel ennExecutePerfSetting;
  }PerfProfileExec;
};

/**
*	@brief Describes the Buffer Handling Mode for Snpe
*/
enum class SnapBufferHandlingMode {
  /**
   * Input/Output mem management done by snap
   */
  DEFAULT,
  /**
   * Input buffer is allocated by usecase as normal buffer and Output is also allocated by usecase
   */
  CLIENT_IN_OUT,
  /**
   * Input buffer is allocated by usecase as ION buffer and Output is also allocated by usecase
   */
  CLIENT_ION_IN_OUT,

  MAXENUM,
};

/**
 *	@brief struct that contains the various options to be passed to a
 *session
 */
struct SnapOptions {
  SnapOptions()
      : compUnit(ComputeUnit::MAXENUM), execType(ExecutionDataType::MAXENUM),
        mType(ModelType::MAXENUM), inputNames({}), outputNames({}), scale({}),
        mean({}), stddev({}), model_file(""), weights_file(""),
        cpuThreadCount(0), allowReshape(false) {}

  ComputeUnit compUnit;

  ExecutionDataType execType;

  ModelType mType;

  /**
   *	@brief input layer name for the model to be executed
   */
  std::vector<std::string> inputNames;

  /**
   *	@brief output layer name for the model to be executed
   */
  std::vector<std::string> outputNames;

  /*
   *	@brief (Optional) Vector of scale values for each input channel
   *to be specified here.
   *     Each input data value will be multiplied by corresponding scale
   *value.
   *     Not used by default.
   */
  std::vector<float> scale;

  /**
   *	@brief (Optional) Vector of mean values for each input channel
   *to be specified here.
   *     Each input data value will be subtracted by corresponding mean
   *value.
   *	This will be overridden if mean_file parameter is specified. Not
   *used by default.
   */
  std::vector<float> mean;

  /**
   *	@brief (Optional) Vector of standard deviation values for each
   *input channel to be specified here.
   *     Each input data value will be divided by corresponding standard
   *deviation value.
   *	Not used by default.
   */
  std::vector<float> stddev;

  /**
   *   @brief Path to the model file .
   */
  std::string model_file;

  /**
   *	@brief Path to the weights file (.caffemodel) should be
   *specified here.
   */
  std::string weights_file;

  /**
   *	@brief The number of inputs of the model should be specified
   *here.
   *	Default value of 1.
   */
  int cpuThreadCount;

  /**
   *	@brief (Optional) Should be set to true to enable support for
   *different input image height and width.
   *	Model should be able to support this reshape, otherwise a crash
   *can occur during Execute().
   *     Not enabled by default.
   */
  bool allowReshape;
};

struct SnapOptionsV1 : SnapOptions {
  SnapOptionsV1()
      : model_buffer({}), enable_secure(false), gpuCacheBuffer(nullptr) {}

  std::vector<uint8_t> model_buffer;
  bool enable_secure;
  char *gpuCacheBuffer;
};

struct SnapOptionsV2 : SnapOptionsV1 {
  SnapOptionsV2()
      : flag(AddOptions::MAXENUM), model_name(""), input_shape({}),
        mode(Mode::MAXENUM), dvfs_perf(Dvfs::MAXENUM), cacheSize(0),
        model_buffer_ptr(nullptr), model_buffer_len(0) {}

  AddOptions flag;
  std::string model_name;
  std::vector<std::vector<int32_t>> input_shape;
  Mode mode;
  Dvfs dvfs_perf;
  size_t cacheSize;
  uint8_t *model_buffer_ptr; /* This is for optimized implementation. */
  size_t model_buffer_len;
};

struct SnapOptionsV3 : SnapOptionsV2 {
  SnapOptionsV3()
      : security_level(SecurityLevel::NO_SECURITY),
        debug_level(DebugLevel::LEVEL0), eden_config(),
        bypass_eden_service(false), ofi_preset("HS_AFFINITY_MID"),
        user_defined_layer(), snpe_buffer_type(SnpeBufferType::UB_TF8),
        snaplite_catalyst(SnapliteCatalyst::DEFAULT),
        gpu_cache_option(GpuCacheOption::MAXENUM), model_buffer_id(-1) {}

  /* Generic Options common to all backends **/
  SecurityLevel security_level;
  DebugLevel debug_level;

  EdenConfiguration eden_config;

  /* whether the Eden service bypass is needed or not**/
  bool bypass_eden_service;

  /* Option specific to Ofi backend. Allowed values:
   * HS_AFFINITY_MID: Use Middle CPU for DSP service.
   * HS_PERF_MAX: Same as the current HS_AFFINITY_MID. After the DD update is
   *completed, the clock is raised to the maximum. HS_PERF: Same as the current
   *HS_AFFINITY_MID. After the DD update is completed, the clock is raised and
   *operated.
   **/
  std::string ofi_preset;

  /* Options specific to User defined Layer and SNPE backend **/
  UserDefinedLayer user_defined_layer;
  SnpeBufferType snpe_buffer_type;

  /* Options specific to Snaplite backend **/
  SnapliteCatalyst snaplite_catalyst;
  GpuCacheOption gpu_cache_option;

  /* Used to send shared mem identifier */
  int model_buffer_id;
};
struct SnapOptionsV4 : SnapOptionsV3 {
  SnapOptionsV4()
      : outputDataType(ExecutionDataType::MAXENUM),
        execOptMode(ExecuteOptimizeMode::DEFAULT),
        quantInfo({}),
        dequantInfo({}),
        snapTvmQnnCacheOption() {}

  /**
   * Describes the Exectution DataType of Output to be used for SnapSession
   */
  ExecutionDataType outputDataType;
  /**
   * Describes whether client will do shared mem management for input-output or service.
   */
  ExecuteOptimizeMode execOptMode;
  /**
   * Quantization information for input data
   */
  std::vector<QuantDequantInfo> quantInfo;
  /**
   * Dequantization information for input data
   */
  std::vector<QuantDequantInfo> dequantInfo;
  /**
   * Options for SNAP-TVM-QNN backends to support cached model
   */
  SnapTvmQnnCacheOption snapTvmQnnCacheOption;
};

struct SnapOptionsV5 : SnapOptionsV4 {
  SnapOptionsV5()
      : profileLevel({}),
        num_tiles(1),
        bufferHandlingMode(SnapBufferHandlingMode::DEFAULT),
        dataFormat(DataFormat::MAXENUM) {}

  /**
   * Describes the level of Performance profile
   */
  PerfProfileLevel profileLevel;
  /**
   * Describes the number of threads required for Multi tile execution
   */
  int num_tiles;    
  /**
   * Describes the Buffer Handling Mode of Snpe
   */
  SnapBufferHandlingMode bufferHandlingMode;
  /**
   *	@brief dataformat . (NHWC/NCHW). order of N, C, H, W will be
   *according to dataType
   */     
  DataFormat dataFormat;
};

/**
*	@brief Class used for data transfer in/out by a session.
*       Input format should be NHWC while Output format will be same as model
*output format.
*/
struct DataBuffer {
  DataBuffer()
      : data(nullptr),
        shape({}),
        dataType(DataBufferType::MAXENUM),
        dataFormat(DataFormat::MAXENUM) {}       
DataBuffer(void *dataValue, std::vector<int> shapeValue, DataBufferType dataTypeValue, DataFormat dataFormatValue)
      : data(dataValue),
        shape(shapeValue),
        dataType(dataTypeValue),
        dataFormat(dataFormatValue) {}       
  /**
   *	@brief Pointer to the data
   */
  void *data;
  /**
   *	@brief shape of data in NHWC or NCHW format
   */
  std::vector<int> shape;
  /**
   *	@brief type of data present in data of DataBuffer
   */
  DataBufferType dataType;
  /**
   *	@brief dataformat . (NHWC/NCHW). order of N, C, H, W will be
   *according to dataType
   */
  DataFormat dataFormat;
};

/**
*	@brief DataBufferOpt is a optimised databuffer class used for data transfer in/out.
*   Applicable only for libsnap_aidl.snap.samsung.so
*   Usecases can use new Android shared memory apis from Vx session
*   Send buffer id instead of pointer data
*/
struct DataBufferOpt : DataBuffer {
  DataBufferOpt()
          : bufferId(-1),
            quantInfo() {}       
  DataBufferOpt(void *dataValue, std::vector<int> shapeValue, DataBufferType dataTypeValue, DataFormat dataFormatValue, 
             int bufferIdValue, QuantDequantInfo quantInfoValue)
          : DataBuffer(dataValue, shapeValue, dataTypeValue, dataFormatValue),
            bufferId(bufferIdValue),
            quantInfo(quantInfoValue) {}           
  /**
   * @brief Used to send shared mem identifier
   */
  int bufferId;
  /**
   * @brief Quantization information for input data
   */
  QuantDequantInfo quantInfo;
};

struct DataBufferOptFd : DataBufferOpt {
  DataBufferOptFd()
          : fd(-1) {}       
  DataBufferOptFd(void *dataValue, std::vector<int> shapeValue, DataBufferType dataTypeValue, DataFormat dataFormatValue, 
             int bufferIdValue, QuantDequantInfo quantInfoValue, int fdValue)
          : DataBufferOpt(dataValue, shapeValue, dataTypeValue, dataFormatValue, bufferIdValue, quantInfoValue),
            fd(fdValue) {}           
  /**
   * @brief file descriptor for ION buffer input data 
   */
  int fd;
};

struct TensorInfo {
  /**
   *    @brief shape of data in NHWC or NCHW format
   */
  std::vector<int> shape;
  /**
   *    @brief type of data present in data of DataBuffer
   */
  DataBufferType dataType;
  /**
   *    @brief dataformat . (NHWC/NCHW). order of N, C, H, W will be
   *according to dataType
   */
  DataFormat dataFormat;
};

/**
 *	@brief Describes the type of interface in snap::TensorSize
 */
enum class UcIntfType {
  VNDK,
  HIDL,
  SPHAL,
  MAX_ENUM,
};

/* Struct to save the input and o/p shape size
 *  layer name should be filled by HIDL as per snapOptions.outputNames
 *  and each backend fills the size of the layer by multiplying the
 *  dimensions as per layer rank
 */
struct TensorSize {

  /* name of the input or output layer */
  std::string name;

  /* size should be multiplication of dimensions for ex: 4 rank
   * dimension, it would be N*H*W*C * sizeof(flaot) */
  int size;

  /* Supported DataBufferType packed, default FLOAT32 */
  uint32_t supportedTypes = 0;
};

/**
 *	@brief A SnapSession instance allows a caller to execute a Caffe model
 */

class SnapSession {
protected:
  virtual ~SnapSession() {}

public:
  /**
   *	@brief Opens the session using the given options
   *	@param options The const SnapOptions used to configure the
   *session
   */
  virtual ErrCode Open(const struct SnapOptions &options) = 0;
  /**
   *	@brief Executes the model using the given inputs and returns a
   *vector of pointers to the outputs.
   *	       Concurrent access is not supported.
   *	@param const inputs Vector of DataBuffer objects passed by
   *reference that contain input
   *data for each corresponding input blob.
   *            Size depends on the size of the input_names parameter of
   *SnapOptions.
   *	@param outputs Pointer to a vector of DataBuffer objects that
   *will contain output data after successful execution.
   *            Size depends on the num_outputs parameter of SnapOptions.
   */
  virtual ErrCode Execute(const std::vector<DataBuffer> &inputs,
                          std::vector<DataBuffer> *outputs) = 0;
  /**
   *	@brief Closes and releases the instance.
   */
  virtual ErrCode Close() = 0;
  /**
   *	@brief Gets the dimensions of any input blob based on provided
   *arguments.
   *	@param input_blob_index The index of the input blob whose
   *dimensions should be returned.
   *	@param dims Pointer to a vector<int> that will contain the
   *dimensions of the specified input blob after successful execution.
   *	       Dimensions will be in NCHW format.
   */
  virtual ErrCode GetModelInputShape(int inputIndex,
                                     std::vector<int> *shape) = 0;
  //  virtual ErrCode GetOptions(SnapOptions *snapOpt) = 0;
};

/**
 * Describes version of given Compute Unit and Model Type that is returned in
 * getSnapBufferVersion
 */
struct SnapVersionInfo {
  SnapVersionInfo()
      : compUnit(ComputeUnit::MAXENUM), modelType(ModelType::MAXENUM),
        version("") {}
  ComputeUnit compUnit;
  ModelType modelType;
  std::string version;
};

/**
 * Describes the buffer that is passed to GetBufferVersion
 */
struct VersionBuffer {
  VersionBuffer()
      : option(BufferOption::MAXENUM), path(""), buffer(nullptr), size(0),
        fd(0) {}
  BufferOption option;
  /** Model / Cache File Path */
  std::string path;
  /** Model / Cache Buffer */
  uint8_t *buffer;
  /** Buffer Size*/
  int32_t size;
  /** Model / Cache FD */
  int32_t fd;
};

class SnapSessionV1 : public SnapSession {
protected:
  virtual ~SnapSessionV1() {}

public:
  using SnapSession::Open;
  /**
   *	@brief Opens the session using the given options
   *	@param options The const SnapOptions used to configure the
   *session
   */
  virtual ErrCode Open(const struct SnapOptionsV1 &options) = 0;
};

class SnapSessionV2 : public SnapSessionV1 {
protected:
  virtual ~SnapSessionV2() {}

public:
  using SnapSessionV1::Open;
  /**
   * @brief Opens the session using the given options
   * @param options The const SnapOptions used to configure the
   *session
   */
  virtual ErrCode Open(const struct SnapOptionsV2 &options) = 0;
  /**
   * @brief Get the size of each output node present in vector
   * @param TensorSize is a vector of output names & size
   * @param UcIntfType is an enum of interface type
   */
  virtual ErrCode GetOutputSize(std::vector<snap::TensorSize> *tensorSize,
                                snap::UcIntfType intf) = 0;
};

class SnapSessionVx : public SnapSessionV2 {
  protected:
   virtual ~SnapSessionVx() {}

  public:
   using SnapSessionV2::Open;
   using SnapSessionV2::Execute;

  /**
  * @brief Opens the session using the given options
  * @param options The const SnapOptions used to configure the
  *session
  */
  virtual ErrCode Open(const struct SnapOptionsV3 &options) = 0;

  /**
  * @brief Get the updated Gpu cache
  * @param Cache is the Cache structure containing cache data,size
  * @param updateStatus is true if the cache is updated, otherwise old cache is valid
  */
  virtual ErrCode GetUpdatedGpuCache(SerializedGpuCacheBinary *cache_ptr, bool &updateStatus) = 0;

  /**
  * @brief The reshape api -- required for arcsoft -- Not available for usecases
  * @param input_shapes is the input shape passed to the model.
  * @param output_sizes points to the output size of the model.
  */
  virtual ErrCode Reshape(std::vector<std::vector<int>> input_shapes, std::vector<snap::TensorSize>* output_sizes) = 0;

  /**
  * @brief Opens the session using the given options
  * @param options The const SnapOptions used to configure the session
  */
  virtual ErrCode Open(const struct SnapOptionsV4 &options) = 0;

  /**
  * @brief Applicable to libsnap_aidl.snap.samsung.so Allocate Android shared memory for model or input or output buffers
  * model buffer - call before open call
  * input or output buffers - call only once before execute and after open
  * @param in_size is the size of shared mem in bytes which needs to be allocated
  * @param out_unique_id unique identifier returned by the function
  */
  virtual ErrCode AllocateAshmem(size_t in_size, int* out_unique_id) = 0;
  
  /**
  * @brief Applicable to libsnap_aidl.snap.samsung.so map the allocated shared mem and return the pointer back
  * @param in_unique_id unique identifier for shared mem obtained in allocatedashmem api
  * @param out_buffer_to_write use this pointer to write the buffer
  */
  virtual ErrCode GetBufferPtr(int in_unique_id, void* &out_buffer_to_write) = 0;
  
  /**
  * @brief Applicable to libsnap_aidl.snap.samsung.so unmap Android shared memory
  * @param in_unique_id unique identifier shared mem obtained in allocatedashmem api
  */
  virtual ErrCode UnmapPtr(int in_unique_id) = 0; 
  
  /**
  * @brief Applicable to libsnap_aidl.snap.samsung.so deallocate Android shared memory
  * If not called shared mem will automatically deallocated in close call
  * @param in_unique_id unique identifier shared mem obtained in allocatedashmem api
  */
  virtual ErrCode DeallocateAshmem(int in_unique_id) = 0; 
  
  /**
  * @brief Executes the model using the given shared mem inputs and returns a
  * vector of pointers to the outputs. Added support to pass quantisation info 
  *
  * @param const inputs Vector of DataBuffer objects passed by
  * reference that contain input
  * buffer_id i.e shared mem identifier for each corresponding input blob.
  * Size depends on the size of the input_names parameter of SnapOptions.
  * @param outputs Pointer to a vector of DataBuffer objects that
  * will contain output data after successful execution.
  * Size depends on the num_outputs parameter of SnapOptions.
  */
  virtual ErrCode Execute(const std::vector<DataBufferOpt> &inputs,
                          std::vector<DataBufferOpt> *outputs) = 0;

  /**
  * @brief The reshape api -- required for arcsoft -- Not available for usecases
  * @param input_shapes is the input shape passed to the model.
  * @param output_sizes points to the output size of the model.
  */
  virtual ErrCode Reshape(std::vector<std::vector<int>> input_shapes, std::vector<snap::TensorSize>* output_sizes,
                          std::vector<snap::DataFormat> dataFormat) = 0;
  /**
  * @brief Executes the model using the given shared mem inputs and returns a
  * vector of pointers to the outputs. Added support to pass quantisation info 
  *
  * @param const inputs 2D Vector of DataBuffer objects passed by
  * reference that contain input
  * buffer_id i.e shared mem identifier for each corresponding input blob.
  * Size depends on the size of the input_names parameter of SnapOptions.
  * @param outputs Pointer to a 2D vector of DataBuffer objects that
  * will contain output data after successful execution.
  * Size depends on the num_outputs parameter of SnapOptions.
  */
  virtual ErrCode Execute(const std::vector<std::vector<DataBufferOpt>> &inputs,
                          std::vector<std::vector<DataBufferOpt>> *outputs) = 0;

  virtual ErrCode Execute(const std::vector<std::vector<DataBuffer>> &inputs,
                          std::vector<std::vector<DataBuffer>> *outputs) = 0;

  virtual ErrCode Execute(const std::vector<std::vector<DataBufferOptFd>> &inputs,
                          std::vector<std::vector<DataBufferOptFd>> *outputs) = 0;

  virtual ErrCode Execute(const std::vector<DataBufferOptFd> &inputs,
                          std::vector<DataBufferOptFd> *outputs) = 0;

  /**
    * @return version - model Version of currently opened model
  */
  virtual std::string GetModelVersion() = 0;
  /**
    * @brief API to fetch Scale and Offset values of Inputs
  */
  virtual std::vector<QuantDequantInfo> GetInputQuantParams() = 0;
  /**
    * @brief API to fetch Scale and Offset values of Outputs
  */
  virtual std::vector<QuantDequantInfo> GetOutputQuantParams() = 0;
  /**
  * @brief Opens the session using the given options
  * @param options The const SnapOptions used to configure the session
  */
  virtual ErrCode Open(const struct SnapOptionsV5 &options) = 0;
  
};

/**
 *@brief Create a instance of  SnapSession
 *returns pointer to the SnapSession instance created
 */
struct ops_ver {
  std::vector<int> onnx_opset;
  float tf_version;
  float caffe_version;
};
extern "C" SnapSession *CreateSnapSession(void);
extern "C" SnapSession *CreateSnapSecureSession(void);
std::vector<std::string> GetCapability(ops_ver &opset);

/**
 *@brief destroy instance of SnapSession
 *@param SnapSession to be destroyed
 */
extern "C" ErrCode DestroySnapSession(SnapSession *session);
extern "C" ErrCode DestroySnapSecureSession(SnapSession *session);

class SnapInfo {
public:
  virtual std::vector<snap::SnapVersionInfo>
  GetAllSupportedSnapVersions(void) = 0;
  virtual std::string GetSnapVersion(ComputeUnit compUnit,
                                     ModelType modelType) = 0;
  virtual std::string GetSnapBufferVersion(ComputeUnit compUnit,
                                           ModelType modelType,
                                           VersionBuffer versionBuffer) = 0;

protected:
  virtual ~SnapInfo() {}
};

extern "C" SnapInfo *CreateSnapInfo(void);
extern "C" ErrCode DestroySnapInfo(SnapInfo *info);

} // namespace snap
#endif // SNAP_API_INCLUDE_SNAP_INTERFACE_H_
