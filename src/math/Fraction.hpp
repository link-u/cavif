//
// Created by psi on 2022/03/24.
//
#pragma once

#include <cstdint>

namespace {

[[ nodiscard ]] int32_t gcd(int32_t m, int32_t n) {
  if(m < n) {
    std::swap(m, n);
  }
  while (n != 0) {
    auto const r = m % n;
    m = n;
    n = r;
  }
  return m;
}

[[ nodiscard ]] int32_t lcd(int32_t const m, int32_t const n) {
  return m / gcd(m,n) * n;
}

}

struct Fraction {
private:
  int32_t numerator_;
  int32_t denominator_;
public:
  explicit Fraction(int32_t const numerator, int32_t const denominator)
  :numerator_(denominator >= 0 ? numerator : -numerator)
  ,denominator_(std::abs(denominator))
  {
  }

  [[ nodiscard ]] Fraction reduce() const {
    int32_t r = gcd(std::abs(numerator_), std::abs(denominator_));
    return Fraction(numerator_ / r, denominator_ / r);
  }
  [[ nodiscard ]] Fraction minus(Fraction const& b) const {
    int32_t const r = lcd(std::abs(denominator_), std::abs(b.denominator_));
    return Fraction((r / denominator_ * numerator_) - (r / b.denominator_ * b.numerator_), r).reduce();
  }
  [[ nodiscard ]] int32_t numerator() const {
    return numerator_;
  }
  [[ nodiscard ]]int32_t denominator() const {
    return denominator_;
  }
  [[ nodiscard ]] bool isInteger() const {
    return denominator_ == 1;
  }
  [[ nodiscard ]] Fraction div(int32_t const d) const {
    return Fraction(numerator_, denominator_ * d).reduce();
  }
};
