//
// Created by psi on 2020/01/07.
//

#pragma once

#include <avif/av1/Parser.hpp>
#include <avif/FileBox.hpp>

class AVIFBuilder final {
private:
  std::shared_ptr<avif::av1::Parser::Result> obus_;
  avif::FileBox fileBox_{};
public:
  AVIFBuilder() = delete;
  AVIFBuilder(std::shared_ptr<avif::av1::Parser::Result> obus);
  ~AVIFBuilder() noexcept = default;
  AVIFBuilder& operator=(AVIFBuilder const&) = delete;
  AVIFBuilder& operator=(AVIFBuilder&&) = delete;
  AVIFBuilder(AVIFBuilder const&) = delete;
  AVIFBuilder(AVIFBuilder&&) = delete;

public:
  avif::FileBox build();
};


