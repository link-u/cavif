//
// Created by psi on 2020/01/07.
//

#include "AVIFBuilder.hpp"

#include <utility>

AVIFBuilder::AVIFBuilder(std::shared_ptr<avif::av1::Parser::Result> obus)
:obus_(std::move(obus))
{

}

avif::FileBox AVIFBuilder::build() {
  return avif::FileBox();
}
