//
// Created by psi on 2020/01/07.
//

#include "AVIFBuilder.hpp"
#include "Config.hpp"

#include <utility>

AVIFBuilder::AVIFBuilder(Config& config, uint32_t width, uint32_t height)
:config_(config)
,width_(width)
,height_(height)
{
}


avif::FileBox AVIFBuilder::build() {
  if(!this->frame_.has_value()){
    throw std::runtime_error("No primary frame.");
  }
  using namespace avif;
  FileBox& fileBox = this->fileBox_;
  {
    // Fill FileTypeBox.
    FileTypeBox& fileTypeBox = fileBox.fileTypeBox;
    fileTypeBox.majorBrand = "avif";
    fileTypeBox.minorVersion = 0;
    fileTypeBox.compatibleBrands.emplace_back("avif");
    fileTypeBox.compatibleBrands.emplace_back("mif1");
    // https://aomediacodec.github.io/av1-avif/#profiles-constraints
    fileTypeBox.compatibleBrands.emplace_back("miaf");
    // https://aomediacodec.github.io/av1-avif/#baseline-profile
    // The AV1 profile shall be the Main Profile and the level shall be 5.1 or lower.
    // fileTypeBox.compatibleBrands.emplace_back("MA1B");
  }
  this->fillPrimaryFrameInfo(this->frame_.value());

  return this->fileBox_;
}

void AVIFBuilder::fillPrimaryFrameInfo(const AVIFBuilder::Frame& frame) {
  using namespace avif;
  MetaBox& metaBox = this->fileBox_.metaBox;
  metaBox.setFullBoxHeader(0, 0);
  { // fill HandlerBox
    HandlerBox& handlerBox = metaBox.handlerBox;
    handlerBox.setFullBoxHeader(0u, 0u);
    handlerBox.name = "cavif - https://github.com/link-u/cavif";
    handlerBox.handler = "pict";
  }
  { // fill ItemInfoBox
    ItemInfoBox& itemInfoBox = metaBox.itemInfoBox;
    itemInfoBox.setFullBoxHeader(1, 0);
    {
      ItemInfoEntry entry{};
      entry.setFullBoxHeader(2, 0);
      entry.itemID = 1;
      entry.itemType = "av01";
      entry.itemProtectionIndex = 0;
      entry.itemName = "Image";
      itemInfoBox.itemInfos.emplace_back(entry);
    }
  }
  { // fill ItemLocationBox with dummy data.
    ItemLocationBox& locationBox = metaBox.itemLocationBox;
    locationBox.setFullBoxHeader(0, 0);
    locationBox.offsetSize = 0;
    locationBox.lengthSize = 4;
    locationBox.baseOffsetSize = 4;
    locationBox.items.emplace_back(ItemLocationBox::Item{
        .itemID = 1,
        .dataReferenceIndex = 0,
        .baseOffset = 0,// TODO: fill it after.
        .extents = {{
            ItemLocationBox::Item::Extent {
                .extentOffset = 0,
                .extentLength = frame.data().size(),
            }
        }},
    });
  }
  {
    PrimaryItemBox pitm{};
    pitm.itemID = 1;
    metaBox.primaryItemBox = pitm;
  }
  { // fill ItemPropertiesBox
    ItemPropertyAssociation assoc{};
    ItemPropertyAssociation::Item item{};
    item.itemID = 1;
    ItemPropertiesBox& propertiesBox = metaBox.itemPropertiesBox;
    {
      // FIXME(ledyba-z): Is it really correct?
      // https://aomediacodec.github.io/av1-isobmff/#av1sampleentry-semantics
      propertiesBox.propertyContainers.properties.emplace_back(PixelAspectRatioBox{
          .hSpacing = 1,
          .vSpacing = 1,
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry{
          .essential = false,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    if(config_.rotation.has_value()) {
      propertiesBox.propertyContainers.properties.emplace_back( ImageRotationBox {
        .angle = static_cast<uint8_t>(config_.rotation.value())
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    {
      propertiesBox.propertyContainers.properties.emplace_back(ImageSpatialExtentsProperty {
          .imageWidth = width_,
          .imageHeight = height_,
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = false,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    {
      uint8_t bpp = 8;
      if(frame.sequenceHeader().colorConfig.highBitdepth) {
        bpp = 10;
        if(frame.sequenceHeader().colorConfig.twelveBit) {
          bpp = 12;
        }
      }
      propertiesBox.propertyContainers.properties.emplace_back(PixelInformationProperty {
          .bitsPerChannel = {{bpp, bpp, bpp}},
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    {
      propertiesBox.propertyContainers.properties.emplace_back(AV1CodecConfigurationRecordBox {
          .av1Config = AV1CodecConfigurationRecord {
              .marker = true,
              .version = 1,
              .seqProfile = frame.sequenceHeader().seqProfile,
              .seqLevelIdx0 = frame.sequenceHeader().operatingPoints.at(0).seqLevelIdx,
              .seqTier0 = frame.sequenceHeader().operatingPoints.at(0).seqTier,
              .highBitDepth = frame.sequenceHeader().colorConfig.highBitdepth,
              .twelveBit = frame.sequenceHeader().colorConfig.twelveBit,
              .monochrome = frame.sequenceHeader().colorConfig.monochrome,
              .chromaSubsamplingX = frame.sequenceHeader().colorConfig.subsamplingX,
              .chromaSubsamplingY = frame.sequenceHeader().colorConfig.subsamplingY,
              .chromaSamplePosition = frame.sequenceHeader().colorConfig.chromaSamplePosition.has_value() ?
                                      frame.sequenceHeader().colorConfig.chromaSamplePosition.value() :
                                      static_cast<uint8_t>(0) /* CSP_UNKNOWN: 6.4.2. Color config semantics */,
              .configOBUs = frame.configOBU(),
          },
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    propertiesBox.associations.emplace_back(assoc);
  }
  this->fileBox_.mediaDataBoxes.push_back(MediaDataBox {
    .offset = 0, // TODO: fill it later.
    .size = frame.data().size(),
  });
}

AVIFBuilder& AVIFBuilder::setPrimaryFrame(AVIFBuilder::Frame&& frame) {
  this->frame_= std::move(frame);
  return *this;
}
