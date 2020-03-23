//
// Created by psi on 2020/01/07.
//

#pragma once

#include <avif/av1/Parser.hpp>
#include <avif/FileBox.hpp>

class Config;
class AVIFBuilder final {
public:
  class Frame final {
  private:
    avif::img::ColorProfile colorProfile_;
    avif::av1::SequenceHeader sequenceHeader_;
    std::vector<uint8_t> configOBU_;
    std::vector<uint8_t> data_;
  public:
    Frame() = delete;
    explicit Frame(avif::img::ColorProfile colorProfile, avif::av1::SequenceHeader sequenceHeader, std::vector<uint8_t> configOBU, std::vector<uint8_t> data)
    :colorProfile_(std::move(colorProfile))
    ,sequenceHeader_(sequenceHeader)
    ,configOBU_(std::move(configOBU))
    ,data_(std::move(data))
    {
    }
    static Frame load(avif::util::Logger& log, std::string const& path);
  public:
    [[ nodiscard ]] avif::img::ColorProfile const& colorProfile() const { return this->colorProfile_; }
    [[ nodiscard ]] avif::av1::SequenceHeader const& sequenceHeader() const { return this->sequenceHeader_; }
    [[ nodiscard ]] std::vector<uint8_t> const& configOBU() const { return this->configOBU_; }
    [[ nodiscard ]] std::vector<uint8_t> const& data() const { return this->data_; }
  };

private:
  avif::util::Logger& log_;
  Config& config_;
  uint32_t const width_;
  uint32_t const height_;
  std::optional<Frame> frame_{};
  std::optional<Frame> alpha_{};
  std::optional<Frame> depth_{};
  avif::FileBox fileBox_{};

public:
  AVIFBuilder() = delete;
  explicit AVIFBuilder(avif::util::Logger& log, Config& config, uint32_t width, uint32_t height);
  ~AVIFBuilder() noexcept = default;
  AVIFBuilder& operator=(AVIFBuilder const&) = delete;
  AVIFBuilder& operator=(AVIFBuilder&&) = delete;
  AVIFBuilder(AVIFBuilder const&) = delete;
  AVIFBuilder(AVIFBuilder&&) = delete;

public:
  AVIFBuilder& setPrimaryFrame(Frame&& frame);
  AVIFBuilder& setAlphaFrame(Frame&& frame);
  AVIFBuilder& setDepthFrame(Frame&& frame);
  [[ nodiscard ]] std::vector<uint8_t> build();

private:
  [[ nodiscard ]] avif::FileBox buildFileBox();
  void linkAuxImages(const uint32_t from, const uint32_t to);
  void fillFrameInfo(uint16_t const itemID, AVIFBuilder::Frame const& frame, std::optional<std::string> const& auxType = {});
};


