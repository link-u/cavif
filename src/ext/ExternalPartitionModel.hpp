//
// Created by psi on 2021/12/04.
//

#pragma once

#include <memory>
#include <aom/aom_external_partition.h>

class ExternalPartitionModel {
public:
  virtual ~ExternalPartitionModel() noexcept = 0;
public:
  virtual void sendFeatures(aom_partition_features_t const* features) = 0;
  virtual void getDecision(aom_partition_decision_t *decision) = 0;
  virtual void sendPartitionStats(aom_partition_stats_t const* stats) = 0;
};

class ExternalPartitionModelFactory {
public:
  virtual ~ExternalPartitionModelFactory() noexcept = 0;
public:
  virtual ExternalPartitionModel* create(aom_ext_part_config_t const* part_config) = 0;
  virtual aom_ext_part_decision_mode_t decisionMode() = 0;
};
