#include <iostream>

#include "../external/clipp/include/clipp.h"
#include <aom/aom.h>
#include <aom/aom_encoder.h>
#include <aom/aom_codec.h>
#include <common/tools_common.h>
#include <aom/aomcx.h>

namespace {

std::string basename(std::string const& path) {
  auto pos = path.find_last_of('/');
  if(pos == std::string::npos) {
    return path;
  }
  return path.substr(pos+1);
}

}

int main(int argc, char** argv) {
  std::string inputFilename = {};
  std::string outputFilename = {};
  aom_codec_ctx_t codec = {};
  aom_codec_enc_cfg cfg = {};
  aom_codec_iface_t* av1codec = aom_codec_av1_cx();
  if(!av1codec){
    std::cerr << ("Failed to get AV1 encoder.");
  }
  aom_codec_enc_config_default(av1codec, &cfg, 0);
  {
    using namespace clipp;
    auto cli = (
        required("-i", "--input") & value("input.{png, bmp}", inputFilename),
        required("-o", "--output") & value("output.avif", outputFilename),
        option("--cfg.encoder_cfg.disable_cdef").set(cfg.encoder_cfg.disable_cdef, 1u),
        option("--cfg.monochrome").set(cfg.monochrome, 1u)
    );
    if(!parse(argc, argv, cli)) {
      std::cerr << make_man_page(cli, basename(std::string(argv[0])));
      return -1;
    }
    if(inputFilename == outputFilename) {
      std::cerr << make_man_page(cli, basename(std::string(argv[0])));
      return -1;
    }
  }
  if(AOM_CODEC_OK != aom_codec_enc_init(&codec, av1codec, &cfg, 0)){

  }
  aom_image_t img;
  aom_img_alloc(&img, AOM_IMG_FMT_I420, 100, 100, 1);

  aom_codec_cx_pkt_t const* pkt;
  aom_codec_iter_t iter = NULL;
  aom_codec_encode(&codec, &img, 0, 0, 0);
  while ((pkt = aom_codec_get_cx_data(&codec, &iter)) != NULL) {
    if (pkt->kind == AOM_CODEC_CX_FRAME_PKT) {
      const int keyframe = (pkt->data.frame.flags & AOM_FRAME_IS_KEY) != 0;
      // write pkt.data
    }
    break;
  }
  bool cont = true;
  while(cont) {
    aom_codec_encode(&codec, nullptr, -1, 0, 0);
    while (aom_codec_get_cx_data(&codec, &iter) == nullptr) {
      cont = false;
    }
  }

  aom_img_free(&img);
  if (aom_codec_destroy(&codec) != AOM_CODEC_OK) {
    die_codec(&codec, "Failed to destroy codec.");
  }

  return 0;
}
