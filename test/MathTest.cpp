//
// Created by psi on 2022/03/27.
//

#include <gtest/gtest.h>
#include "../src/math/Fraction.hpp"

TEST(MathTest, GCD) {
  ASSERT_EQ(10, gcd(10, 20));
  ASSERT_EQ(10, gcd(20, 10));
  ASSERT_EQ(2, gcd(10, 2));
  ASSERT_EQ(1, gcd(10, 3));
  ASSERT_EQ(1, gcd(114514, 997));
}

TEST(MathTest, LCD) {
  ASSERT_EQ(20, lcd(10, 20));
  ASSERT_EQ(20, lcd(20, 10));
  ASSERT_EQ(10, lcd(10, 2));
  ASSERT_EQ(30, lcd(10, 3));
  ASSERT_EQ(114514 * 997, lcd(114514, 997));
}

