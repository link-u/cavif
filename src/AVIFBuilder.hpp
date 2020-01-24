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
    avif::av1::SequenceHeader sequenceHeader_;
    std::vector<uint8_t> configOBU_;
    std::vector<uint8_t> data_;
  public:
    Frame() = delete;
    explicit Frame(avif::av1::SequenceHeader sequenceHeader, std::vector<uint8_t> configOBU, std::vector<uint8_t> data)
    :sequenceHeader_(sequenceHeader)
    ,configOBU_(std::move(configOBU))
    ,data_(std::move(data))
    {
    }
  public:
    [[ nodiscard ]] avif::av1::SequenceHeader const& sequenceHeader() const { return this->sequenceHeader_; }
    [[ nodiscard ]] std::vector<uint8_t> const& configOBU() const { return this->configOBU_; }
    [[ nodiscard ]] std::vector<uint8_t> const& data() const { return this->data_; }
  };

private:
private:
  Config& config_;
  uint32_t const width_;
  uint32_t const height_;
  std::optional<Frame> frame_{};
  avif::FileBox fileBox_{};

public:
  AVIFBuilder() = delete;
  explicit AVIFBuilder(Config& config, uint32_t width, uint32_t height);
  ~AVIFBuilder() noexcept = default;
  AVIFBuilder& operator=(AVIFBuilder const&) = delete;
  AVIFBuilder& operator=(AVIFBuilder&&) = delete;
  AVIFBuilder(AVIFBuilder const&) = delete;
  AVIFBuilder(AVIFBuilder&&) = delete;

public:
  AVIFBuilder& setPrimaryFrame(Frame&& frame);
  [[ nodiscard ]] avif::FileBox build();

  void fillPrimaryFrameInfo(Frame const& frame);
};


