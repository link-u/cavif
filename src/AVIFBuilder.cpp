//
// Created by psi on 2020/01/07.
//

#include <utility>
#include <memory>

#include <avif/util/StreamWriter.hpp>
#include <avif/util/File.hpp>

#include <avif/Constants.hpp>

#include <avif/Writer.hpp>
#include <avif/Parser.hpp>
#include <avif/Query.hpp>

#include <avif/av1/Parser.hpp>
#include <avif/av1/Query.hpp>
#include <avif/util/FourCC.hpp>

#include "AVIFBuilder.hpp"
#include "Config.hpp"

namespace {

enum AVIFProfile : uint8_t {
  Baseline = 0,
  Advanced = 1,
  Unspecified = 255,
};

AVIFProfile calcProfile(AVIFBuilder::Frame& frame) {
  // https://aomediacodec.github.io/av1-avif/#baseline-profile
  // The AV1 profile shall be the Main Profile and the level shall be 5.1 or lower.
  if(frame.sequenceHeader().seqProfile == 0) {
    bool baseProfile = true;
    for(auto const& operatingPoint : frame.sequenceHeader().operatingPoints) {
      baseProfile &= operatingPoint.seqLevelIdx <= 13;
    }
    if (baseProfile) {
      return Baseline;
    }
  }
  // https://aomediacodec.github.io/av1-avif/#advanced-profile
  // 6.4. AVIF Advanced Profile
  // The AV1 profile shall be the High Profile and the level shall be 6.0 or lower.
  if(frame.sequenceHeader().seqProfile == 1) {
    bool advancedProfile = true;
    for(auto const& operatingPoint : frame.sequenceHeader().operatingPoints) {
      advancedProfile &= operatingPoint.seqLevelIdx <= 16;
    }
    if (advancedProfile) {
      return Advanced;
    }
  }
  return Unspecified;
}


}

AVIFBuilder::Frame AVIFBuilder::Frame::load(avif::util::Logger& log, std::string const& path) {
  std::variant<std::vector<uint8_t>, std::string> file = avif::util::readFile(path);
  if(std::holds_alternative<std::string>(file)) {
    log.fatal(std::get<std::string>(file));
  }
  std::shared_ptr<avif::Parser::Result> result = avif::Parser(log, std::get<std::vector<uint8_t>>(file)).parse();
  if(!result->ok()) {
    log.fatal(result->error());
  }
  avif::FileBox const& fileBox = result->fileBox();
  std::optional<uint32_t> primaryItemID{};
  if(!fileBox.metaBox.primaryItemBox.has_value()) {
    primaryItemID = fileBox.metaBox.primaryItemBox.value().itemID;
  }
  namespace query = avif::util::query;
  avif::img::ColorProfile colorProfile;
  std::optional<avif::ColourInformationBox> colr = query::findProperty<avif::ColourInformationBox>(fileBox, primaryItemID);
  if(colr.has_value()) {
    auto profile = colr.value().profile;
    if(std::holds_alternative<avif::ColourInformationBox::RestrictedICC>(profile)) {
      colorProfile = avif::img::ICCProfile(std::get<avif::ColourInformationBox::RestrictedICC>(profile).payload);
    } else if(std::holds_alternative<avif::ColourInformationBox::UnrestrictedICC>(profile)) {
      colorProfile = avif::img::ICCProfile(std::get<avif::ColourInformationBox::UnrestrictedICC>(profile).payload);
    }
  }

  std::optional<avif::AV1CodecConfigurationRecordBox> av1Config = query::findProperty<avif::AV1CodecConfigurationRecordBox>(fileBox, primaryItemID);
  if(!av1Config.has_value()) {
    log.fatal("AV1 config not found: {}", path);
  }
  std::vector<uint8_t> configOBUs = av1Config.value().av1Config.configOBUs;
  std::shared_ptr<avif::av1::Parser::Result> av1result = avif::av1::Parser(log, configOBUs).parse();
  if(!av1result->ok()) {
    log.fatal(av1result->error());
  }
  std::pair<size_t, size_t> region = query::findItemRegion(fileBox, primaryItemID);
  auto beg = std::begin(result->buffer());
  std::vector<uint8_t> mdat = std::vector<uint8_t>(std::next(beg, region.first), std::next(beg, region.second));
  namespace av1query = avif::av1::util::query;
  std::optional<avif::av1::SequenceHeader> seq = av1query::find<avif::av1::SequenceHeader>(av1result->packets());
  if(!seq.has_value()) {
    log.fatal("Sequence header not found.");
  }
  return AVIFBuilder::Frame(colorProfile, std::move(seq.value()), std::move(configOBUs), std::move(mdat));
}

AVIFBuilder::AVIFBuilder(avif::util::Logger& log, Config& config, uint32_t width, uint32_t height)
:log_(log)
,config_(config)
,width_(width)
,height_(height)
{
}

avif::FileBox AVIFBuilder::buildFileBox() {
  if(!this->frame_.has_value()){
    throw std::runtime_error("No primary frame.");
  }
  Frame& frame = this->frame_.value();
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
    AVIFProfile profile = calcProfile(frame);
    if(this->alpha_.has_value()) {
      AVIFProfile alphaProfile = calcProfile(this->alpha_.value());
      if(alphaProfile != profile) {
        profile = Unspecified;
      }
    }
    if(this->depth_.has_value()) {
      AVIFProfile depthProfile = calcProfile(this->depth_.value());
      if(depthProfile != profile) {
        profile = Unspecified;
      }
    }
    // Set a maximum compatible profile.
    if(profile == Baseline) {
      fileTypeBox.compatibleBrands.emplace_back("MA1B");
    }else if(profile == Advanced) {
      fileTypeBox.compatibleBrands.emplace_back("MA1A");
    }
  }
  {
    // Fill metabox
    MetaBox& metaBox = this->fileBox_.metaBox;
    metaBox.setFullBoxHeader(0, 0);
    { // fill HandlerBox
      HandlerBox& handlerBox = metaBox.handlerBox;
      handlerBox.setFullBoxHeader(0u, 0u);
      handlerBox.name = "cavif";
      handlerBox.handler = "pict";
    }
  }
  this->fillFrameInfo(1, frame);
  size_t nextID = 2;
  if(this->alpha_.has_value()) {
    this->fillFrameInfo(nextID, alpha_.value(), avif::kAlphaAuxType());
    this->linkAuxImages(nextID, 1);
    ++nextID;
  }
  if(this->depth_.has_value()) {
    this->fillFrameInfo(nextID, depth_.value(), avif::kDepthAuxType());
    this->linkAuxImages(nextID, 1);
    ++nextID;
  }
  return this->fileBox_;
}

void AVIFBuilder::linkAuxImages(uint32_t const from, uint32_t const to) {
  using namespace avif;
  if(!this->fileBox_.metaBox.itemReferenceBox.has_value()) {
    ItemReferenceBox box{
        .references = std::vector<SingleItemTypeReferenceBoxLarge>{},
    };
    box.setFullBoxHeader(1, 0);
    this->fileBox_.metaBox.itemReferenceBox = box;
  }
  auto& refs = std::get<std::vector<SingleItemTypeReferenceBoxLarge>>(this->fileBox_.metaBox.itemReferenceBox.value().references);
  for(auto& ref : refs) {
    if(ref.fromItemID == from) {
      for(auto const& oldTo : ref.toItemIDs) {
        if(oldTo == to) {
          return;
        }
      }
      ref.toItemIDs.emplace_back(to);
      return;
    }
  }
  auto box = SingleItemTypeReferenceBoxLarge{
      .fromItemID = from,
      .toItemIDs = {to},
  };
  box.hdr.type = avif::util::str2uint("auxl");
  refs.emplace_back(box);
}

std::vector<uint8_t> AVIFBuilder::build() {
  avif::FileBox fileBox = this->buildFileBox();
  { // pass1: calc positions
    avif::util::StreamWriter pass1;
    avif::Writer(log_, pass1).write(fileBox);
  }
  for (size_t i = 0; i < fileBox.metaBox.itemLocationBox.items.size(); ++i) {
    size_t const offset = fileBox.mediaDataBoxes.at(i).offset;
    fileBox.metaBox.itemLocationBox.items.at(i).baseOffset = offset;
  }
  avif::util::StreamWriter out;
  avif::Writer(log_, out).write(fileBox);
  std::vector<uint8_t> data = out.buffer();
  auto beg = std::begin(data);
  if(frame_.has_value()) {
    Frame& frame = frame_.value();
    std::copy(std::begin(frame.data()), std::end(frame.data()), std::next(beg, fileBox.mediaDataBoxes.at(0).offset));
  }
  if(alpha_.has_value()){
    Frame& frame = alpha_.value();
    std::copy(std::begin(frame.data()), std::end(frame.data()), std::next(beg, fileBox.mediaDataBoxes.at(1).offset));
  }
  return data;
}

void AVIFBuilder::fillFrameInfo(uint16_t const itemID, AVIFBuilder::Frame const& frame, std::optional<std::string> const& auxType) {
  using namespace avif;
  MetaBox& metaBox = this->fileBox_.metaBox;
  { // fill ItemInfoBox
    ItemInfoBox& itemInfoBox = metaBox.itemInfoBox;
    itemInfoBox.setFullBoxHeader(1, 0);
    {
      ItemInfoEntry entry{};
      entry.setFullBoxHeader(2, 0);
      entry.itemID = itemID;
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
        .itemID = itemID,
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
  if(!auxType.has_value()) { // Primary Image!
    PrimaryItemBox pitm{};
    pitm.itemID = itemID;
    metaBox.primaryItemBox = pitm;
  }
  { // fill ItemPropertiesBox
    ItemPropertyAssociation assoc{};
    ItemPropertyAssociation::Item item{};
    item.itemID = itemID;
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
    if(config_.cropSize.has_value() || config_.cropOffset.has_value()) {
      CleanApertureBox clap{};
      if(config_.cropSize.has_value()) {
        clap.cleanApertureWidthN = config_.cropSize.value().first.first;
        clap.cleanApertureWidthD = config_.cropSize.value().first.second;
        clap.cleanApertureHeightN = config_.cropSize.value().second.first;
        clap.cleanApertureHeightD = config_.cropSize.value().second.second;
      }
      if(config_.cropOffset.has_value()) {
        clap.horizOffN = config_.cropOffset.value().first.first;
        clap.horizOffD = config_.cropOffset.value().first.second;
        clap.vertOffN = config_.cropOffset.value().second.first;
        clap.vertOffD = config_.cropOffset.value().second.second;
      }
      propertiesBox.propertyContainers.properties.emplace_back(clap);
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    if(config_.rotation.has_value()) {
      propertiesBox.propertyContainers.properties.emplace_back( ImageRotationBox {
          .angle = config_.rotation.value(),
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    if(config_.mirrorAxis.has_value()) {
      propertiesBox.propertyContainers.properties.emplace_back( ImageMirrorBox {
          .axis = config_.mirrorAxis.value(),
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
      if(frame.sequenceHeader().colorConfig.monochrome) {
        propertiesBox.propertyContainers.properties.emplace_back(PixelInformationProperty {
            .bitsPerChannel = {bpp},
        });
      } else {
        propertiesBox.propertyContainers.properties.emplace_back(PixelInformationProperty {
            .bitsPerChannel = {bpp, bpp, bpp},
        });
      }
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    {
      if(std::holds_alternative<avif::img::ICCProfile>(frame.colorProfile())) {
        auto const& icc = std::get<avif::img::ICCProfile>(frame.colorProfile());
        propertiesBox.propertyContainers.properties.emplace_back(ColourInformationBox {
            .profile = ColourInformationBox::UnrestrictedICC{
                .payload = icc.payload(),
            }
        });
      }
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = false,
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
    if(auxType.has_value()) {
      propertiesBox.propertyContainers.properties.emplace_back(AuxiliaryTypeProperty {
        .auxType = auxType.value(),
      });
      item.entries.emplace_back(ItemPropertyAssociation::Item::Entry {
          .essential = true,
          .propertyIndex = static_cast<uint16_t>(propertiesBox.propertyContainers.properties.size()),
      });
    }
    assoc.items.emplace_back(item);
    propertiesBox.associations.emplace_back(assoc);
  }
  this->fileBox_.mediaDataBoxes.push_back(MediaDataBox {
    .offset = 0, // TODO: fill it later.
    .size = frame.data().size(),
  });
}


AVIFBuilder& AVIFBuilder::setPrimaryFrame(AVIFBuilder::Frame&& frame) {
  this->frame_ = std::move(frame);
  return *this;
}

AVIFBuilder& AVIFBuilder::setAlphaFrame(AVIFBuilder::Frame&& frame) {
  this->alpha_ = std::move(frame);
  return *this;
}

AVIFBuilder& AVIFBuilder::setDepthFrame(AVIFBuilder::Frame&& frame) {
  this->depth_ = std::move(frame);
  return *this;
}
