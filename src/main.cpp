#include <iostream>

#include "../external/clipp/include/clipp.h"

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
  {
    using namespace clipp;
    auto cli = (
        required("-i", "--input") & value("input.{png, bmp}", inputFilename),
        required("-o", "--output") & value("output.avif", outputFilename)
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
  return 0;
}
