//
// Created by psi on 2020/01/07.
//

#include "AVIFBuilder.hpp"

#include <utility>

AVIFBuilder::AVIFBuilder(uint32_t width, uint32_t height)
:width_(width)
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
    locationBox.itemCount = 1;
    locationBox.items.emplace_back(ItemLocationBox::Item{
        .itemID = 1,
        .dataReferenceIndex = 0,
        .baseOffset = 0,// TODO: fill it after.
        .extentCount = 1,
    });
    locationBox.items[0].extents.emplace_back(ItemLocationBox::Item::Extent{
        .extentOffset = 0,
        .extentLength = frame.data().size(),
    });
  }
  { // fill ItemPropertiesBox
    ItemPropertiesBox& propertiesBox = metaBox.itemPropertiesBox;
    // FIXME(ledyba-z):
    // https://aomediacodec.github.io/av1-isobmff/#av1sampleentry-semantics
    propertiesBox.itemPropertyContainer.properties.emplace_back(PixelAspectRatioBox{
        .hSpacing = 1,
        .vSpacing = 1,
    });
    propertiesBox.itemPropertyContainer.properties.emplace_back(ImageSpatialExtentsProperty{
        .imageWidth = width_,
        .imageHeight = height_,
    });
    uint8_t bpp = 8;
    if(frame.sequenceHeader().colorConfig.highBitdepth) {
      bpp = 10;
      if(frame.sequenceHeader().colorConfig.twelveBit) {
        bpp = 12;
      }
    }
    propertiesBox.itemPropertyContainer.properties.emplace_back(PixelInformationProperty{
        .numChannels = 3,
        .bitsPerChannel = {{bpp, bpp, bpp}},
    });
    propertiesBox.itemPropertyContainer.properties.emplace_back(AV1CodecConfigurationRecordBox{
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
    propertiesBox.itemPropertyAssociations.emplace_back(ItemPropertyAssociation{
      .itemCount = 1,
      .items = {{
          ItemPropertyAssociation::Item {
            .itemID = 1,
            .entryCount = 4,
            .entries = {{
                ItemPropertyAssociation::Item::Entry {
                    .essential = false,
                    .propertyIndex = 1,
                },
                ItemPropertyAssociation::Item::Entry {
                    .essential = false,
                    .propertyIndex = 2,
                },
                ItemPropertyAssociation::Item::Entry {
                    .essential = true,
                    .propertyIndex = 3,
                },
                ItemPropertyAssociation::Item::Entry {
                    .essential = true,
                    .propertyIndex = 4,
                },
            }},
          }},
      },
    });
  }
  this->fileBox_.mediaDataBoxes.push_back(MediaDataBox {
    .beg = 0, // TODO: fill it later.
    .end = 0, // TODO: fill it later.
  });
}

AVIFBuilder& AVIFBuilder::setFrame(AVIFBuilder::Frame&& frame) {
  this->frame_= std::move(frame);
  return *this;
}
