#include "custom_engine.hpp"

int main(int argc, char* argv[]) {

  Engine engine;
  engine.init();
  engine.mainLoop();
  engine.cleanUp();

  return 0;
}