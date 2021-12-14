//
// Created by psi on 2021/12/11.
//

#pragma once

#include "../ExternalPartitionModel.hpp"

class NonSplitPartitionModel final : public ExternalPartitionModel{
private:
  aom_ext_part_config_t const partitionConfig_;
public:
  NonSplitPartitionModel(aom_ext_part_config_t const* part_config);
public:
  ~NonSplitPartitionModel() noexcept override;

  void sendFeatures(aom_partition_features_t const* features) override;

  void getDecision(aom_partition_decision_t* decision) override;

  void sendPartitionStats(aom_partition_stats_t const* stats) override;
};

class NonSplitPartitionModelFactory final: public ExternalPartitionModelFactory {
public:
  ~NonSplitPartitionModelFactory() noexcept override;
  ExternalPartitionModel* create(aom_ext_part_config_t const* part_config) override;
  aom_ext_part_decision_mode_t decisionMode() override;
};
