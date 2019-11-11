#ifndef ONEFLOW_XRT_TENSORRT_TRT_BUILDER_H_
#define ONEFLOW_XRT_TENSORRT_TRT_BUILDER_H_

#include "NvInfer.h"
#include "glog/logging.h"

#include "oneflow/xrt/parameter.h"
#include "oneflow/xrt/tensorrt/trt_logger.h"
#include "oneflow/xrt/tensorrt/trt_shape.h"
#include "oneflow/xrt/tensorrt/trt_unique_ptr.h"
#include "oneflow/xrt/utility/stl.h"

namespace oneflow {
namespace xrt {
namespace tensorrt {

enum class TrtValueKind : int {
  kUndef = 0,
  kTensor = 1,
  kWeight = 2,
};

inline bool IsUndefKind(const TrtValueKind &kind) {
  return kind == TrtValueKind::kUndef;
}

inline bool IsTensorKind(const TrtValueKind &kind) {
  return kind == TrtValueKind::kTensor;
}

inline bool IsWeightKind(const TrtValueKind &kind) {
  return kind == TrtValueKind::kWeight;
}

#define FOREACH_TENSORRT_LAYER(__macro) \
  __macro(Input);                       \
  __macro(Convolution);                 \
  __macro(FullyConnected);              \
  __macro(Activation);                  \
  __macro(Pooling);                     \
  __macro(LRN);                         \
  __macro(Scale);                       \
  __macro(SoftMax);                     \
  __macro(Concatenation);               \
  __macro(Deconvolution);               \
  __macro(ElementWise);                 \
  __macro(Unary);                       \
  __macro(Padding);                     \
  __macro(Shuffle);                     \
  __macro(Reduce);                      \
  __macro(TopK);                        \
  __macro(Gather);                      \
  __macro(RaggedSoftMax);               \
  __macro(MatrixMultiply);              \
  __macro(Constant);                    \
  __macro(RNNv2);                       \
  __macro(Identity);                    \
  __macro(PluginV2);                    \
  __macro(Slice);                       \
  __macro(Shape);                       \
  __macro(ParametricReLU);              \
  __macro(ConvolutionNd);               \
  __macro(PoolingNd);                   \
  __macro(DeconvolutionNd);             \
  __macro(ScaleNd);                     \
  __macro(Resize);

// Wrapper of tensorrt builder and network.
class TrtBuilder {
 private:
  std::string builder_name_;

  // The next new handle number.
  int64_t next_handle_ = 0;

  nv::unique_ptr<nvinfer1::IBuilder> builder_;
  nv::unique_ptr<nvinfer1::INetworkDefinition> network_;

  util::Map<int64_t, TrtValueKind> value_kinds_;
  util::Map<int64_t, Parameter> params_;
  util::Map<int64_t, nvinfer1::ITensor *> tensors_;
  util::Map<int64_t, nvinfer1::Weights> weights_;

 public:
  explicit TrtBuilder(const std::string &name)
      : builder_name_(name), next_handle_(0) {
    nv::Logger logger(name);
    builder_.reset(nvinfer1::createInferBuilder(logger));
    network_.reset(builder_->createNetwork());
  }

  nvinfer1::ITensor *GetTensor(int64_t handle);

  nvinfer1::Weights &GetWeight(int64_t handle);

  const TrtValueKind &ValueKind(int64_t handle) const {
    CHECK_GT(value_kinds_.count(handle), 0)
        << "Handle " << handle << " has not been built for this builder.";
    return value_kinds_.at(handle);
  }

  nvinfer1::IBuilder *builder() const { return builder_.get(); }

  nvinfer1::INetworkDefinition *network() const { return network_.get(); }

  // Returns handle for the added parameter.
  int64_t AddParameter(const Parameter &param);

  // Returns handle for the added tensor.
  int64_t AddTensor(nvinfer1::ITensor *tensor);

  // Returns handle for the added weight.
  int64_t AddWeight(nvinfer1::Weights &weight);

  void MarkOutput(int64_t handle) {
    nvinfer1::ITensor *output = GetTensor(handle);
    network_->markOutput(*output);
  }

  nv::unique_ptr<nvinfer1::ICudaEngine> BuildCudaEngine() {
    return nv::unique_ptr<nvinfer1::ICudaEngine>(
        builder_->buildCudaEngine(*network_));
  }

#define TRT_BUILDER_ADD_LAYER(Layer)                                          \
  template <typename... Args>                                                 \
  auto add##Layer(Args &&... args)->decltype(network_->add##Layer(args...)) { \
    return network_->addFullyConnected(std::forward<Args>(args)...);          \
  }

  FOREACH_TENSORRT_LAYER(TRT_BUILDER_ADD_LAYER);

 private:
  void CheckHasParameter(int64_t handle) const {
    CHECK_GT(params_.count(handle), 0)
        << "Parameter is not found for handle " << handle;
  }

  int64_t IncreaseHandle() { return next_handle_++; }
};

}  // namespace tensorrt
}  // namespace xrt
}  // namespace oneflow

#endif  // ONEFLOW_XRT_TENSORRT_TRT_BUILDER_H_
