//
// Created by psi on 2021/12/04.
//

#pragma once

#include <memory>
#include <aom/aom_external_partition.h>

class ExternalPartitionModel;
class ExternalPartitionModelFactory;

class ExternalPartitionModelFactoryContainer final {
public:
  explicit ExternalPartitionModelFactoryContainer(std::unique_ptr<ExternalPartitionModelFactory> factory);
  ExternalPartitionModelFactoryContainer() = default;
  ExternalPartitionModelFactoryContainer(ExternalPartitionModelFactoryContainer&&)  noexcept = default;
  ExternalPartitionModelFactoryContainer(ExternalPartitionModelFactoryContainer const&) = delete;
  ExternalPartitionModelFactoryContainer& operator=(ExternalPartitionModelFactoryContainer&&) = default;
  ExternalPartitionModelFactoryContainer& operator=(ExternalPartitionModelFactoryContainer const&) = delete;
  ~ExternalPartitionModelFactoryContainer() noexcept = default;

public:
  aom_ext_part_funcs_t makeBridge();
private:
  std::unique_ptr<ExternalPartitionModelFactory> factory_;
private:
  static aom_ext_part_status_t create_model_(
      void *priv, const aom_ext_part_config_t *part_config,
      aom_ext_part_model_t *ext_part_model);
  static aom_ext_part_status_t delete_model_(aom_ext_part_model_t ext_part_model);

  static aom_ext_part_status_t send_features_(
      aom_ext_part_model_t ext_part_model,
      const aom_partition_features_t *part_features);

  static aom_ext_part_status_t get_partition_decision_(
      aom_ext_part_model_t ext_part_model,
      aom_partition_decision_t *ext_part_decision);

  static aom_ext_part_status_t send_partition_stats_(
      aom_ext_part_model_t ext_part_model,
      const aom_partition_stats_t *ext_part_stats);
};
