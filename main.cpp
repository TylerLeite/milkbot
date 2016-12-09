#include "ai.h"

int main(int argc, char** argv) {

  AI* bestAIEvar = new AI();
  if (argc > 1) {
    if (argc == 3 && (strcmp(argv[23], "-continuous") == 0 || strcmp(argv[2], "-c") == 0)) {
      bestAIEvar->initGame(argv[1], true);
    } else {
      bestAIEvar->initGame(argv[1], false);
    }
  } else {
    std::cout << "Usage: bin/milkbot -MODE -DEPTH -CONTINUOUS?" << std::endl;
  }

  return 0;
}
