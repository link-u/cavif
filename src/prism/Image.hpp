//
// Created by psi on 2020/01/05.
//

#pragma once

#include <vector>
#include <cstdint>

namespace prism {

class Image final {
public:
  enum class Type {
    RGB, /* [R,G,B], [R,G,B], ... */
    RGBA, /* [R,G,B,A], [R,G,B,A], ... */
  };
private:
  Type type_{};
  uint32_t width_{};
  uint32_t height_{};
  uint8_t bitsPerComponent_{};
  uint32_t bytesPerPixel_{};
  uint32_t stride_{};
  std::vector<uint8_t> data_{};
public:
  Image() = default;
  Image(Image const&) = default;
  Image(Image&&) = default;
  Image& operator=(Image const&) = default;
  Image& operator=(Image&&) = default;
  ~Image() noexcept = default;
public:
  explicit Image(Type type, uint32_t width, uint32_t height, uint8_t bitsPerComponent, uint32_t bytesPerPixel, uint32_t stride, std::vector<uint8_t> data);
  [[ nodiscard ]] Type type() const {
    return this->type_;
  }
  [[ nodiscard ]] uint32_t width() const {
    return this->width_;
  }
  [[ nodiscard ]] uint32_t height() const {
    return this->height_;
  }
  [[ nodiscard ]] uint32_t stride() const {
    return this->stride_;
  }
  [[ nodiscard ]] uint32_t bytesPerPixel() const {
    return this->bytesPerPixel_;
  }
  [[ nodiscard ]] uint8_t bitsPerComponent() const {
    return this->bitsPerComponent_;
  }
  template <typename T>
  [[ nodiscard ]] T const* data() const;
};

}
