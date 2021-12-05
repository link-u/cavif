//
// Created by psi on 2021/12/04.
//

#include "ExternalPartitionModelFactoryContainer.hpp"
#include "ExternalPartitionModel.hpp"

ExternalPartitionModelFactoryContainer::ExternalPartitionModelFactoryContainer(std::unique_ptr<ExternalPartitionModelFactory> factory)
:factory_(std::move(factory))
{
}

aom_ext_part_funcs_t ExternalPartitionModelFactoryContainer::makeBridge() {
  aom_ext_part_funcs_t bridge;
  bridge.create_model = ExternalPartitionModelFactoryContainer::create_model_;
  bridge.delete_model = ExternalPartitionModelFactoryContainer::delete_model_;
  bridge.send_features = ExternalPartitionModelFactoryContainer::send_features_;
  bridge.get_partition_decision = ExternalPartitionModelFactoryContainer::get_partition_decision_;
  bridge.send_partition_stats = ExternalPartitionModelFactoryContainer::send_partition_stats_;
  bridge.decision_mode = this->factory_->decisionMode();
  bridge.priv = reinterpret_cast<void*>(this);
  return bridge;
}

aom_ext_part_status_t ExternalPartitionModelFactoryContainer::create_model_(
    void *priv,
    const aom_ext_part_config_t *part_config,
    aom_ext_part_model_t *ext_part_model
) {
  auto self = reinterpret_cast<ExternalPartitionModelFactoryContainer*>(priv);
  *ext_part_model = reinterpret_cast<aom_ext_part_model_t>(self->factory_->create(part_config));
  return AOM_EXT_PART_OK;
}

aom_ext_part_status_t ExternalPartitionModelFactoryContainer::delete_model_(
    aom_ext_part_model_t ext_part_model)
{
  auto model = reinterpret_cast<ExternalPartitionModel*>(ext_part_model);
  delete model;
  return AOM_EXT_PART_OK;
}

aom_ext_part_status_t ExternalPartitionModelFactoryContainer::send_features_(
    aom_ext_part_model_t ext_part_model,
    const aom_partition_features_t *part_features
) {
  auto model = reinterpret_cast<ExternalPartitionModel*>(ext_part_model);
  model->sendFeatures(part_features);
  return AOM_EXT_PART_OK;
}

aom_ext_part_status_t ExternalPartitionModelFactoryContainer::get_partition_decision_(
    aom_ext_part_model_t ext_part_model,
    aom_partition_decision_t *ext_part_decision)
{
  auto model = reinterpret_cast<ExternalPartitionModel*>(ext_part_model);
  model->getDecision(ext_part_decision);
  return AOM_EXT_PART_OK;
}

aom_ext_part_status_t ExternalPartitionModelFactoryContainer::send_partition_stats_(
    aom_ext_part_model_t ext_part_model,
    const aom_partition_stats_t *ext_part_stats
) {
  auto model = reinterpret_cast<ExternalPartitionModel*>(ext_part_model);
  model->sendPartitionStats(ext_part_stats);
  return AOM_EXT_PART_OK;
}
