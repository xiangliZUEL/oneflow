#ifndef ONEFLOW_CORE_KERNEL_DATA_LOAD_KERNEL_H_
#define ONEFLOW_CORE_KERNEL_DATA_LOAD_KERNEL_H_

#include "oneflow/core/kernel/kernel.h"
#include "oneflow/core/data/data_loader.h"

namespace oneflow {

class DataLoadKernel final : public KernelIf<DeviceType::kCPU> {
 public:
  OF_DISALLOW_COPY_AND_MOVE(DataLoadKernel);
  DataLoadKernel() = default;
  ~DataLoadKernel() = default;

 private:
  void Forward(const KernelCtx& ctx,
               std::function<Blob*(const std::string&)> BnInOp2Blob) const override;
  void VirtualKernelInit() override;

  void WriteDataToBlob(
      DeviceCtx* ctx, const std::vector<std::unique_ptr<data::DataInstance>>& data_inst_vec,
      const BlobConf& blob_conf, Blob* out_blob) const;

  std::unique_ptr<data::DataLoader> data_loader_;
};

}  // namespace oneflow

#endif  // ONEFLOW_CORE_KERNEL_DATA_LOAD_KERNEL_H_
