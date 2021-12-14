//
// Created by psi on 2021/12/11.
//

#include <cstring>
#include "NonSplitPartitionModel.hpp"

NonSplitPartitionModelFactory::~NonSplitPartitionModelFactory() noexcept = default;

ExternalPartitionModel* NonSplitPartitionModelFactory::create(aom_ext_part_config_t const* part_config) {
  return new NonSplitPartitionModel(part_config);
}

aom_ext_part_decision_mode_t NonSplitPartitionModelFactory::decisionMode() {
  return AOM_EXT_PART_WHOLE_TREE;
}

/*
 * NonSplitPartitionModel
 */

NonSplitPartitionModel::NonSplitPartitionModel(aom_ext_part_config_t const* part_config)
:partitionConfig_(*part_config)
{
}

NonSplitPartitionModel::~NonSplitPartitionModel() noexcept = default;

void NonSplitPartitionModel::sendFeatures(aom_partition_features_t const* features) {
  // ignore.
}

void NonSplitPartitionModel::getDecision(aom_partition_decision_t* dst) {
  aom_partition_decision_t decision = {};
  decision.is_final_decision = true;
  *dst = decision;
}

void NonSplitPartitionModel::sendPartitionStats(aom_partition_stats_t const* stats) {
  // ignore.
}
