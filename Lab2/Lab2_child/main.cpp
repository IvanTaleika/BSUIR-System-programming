#include "coffeemachine.h"
#include "human.h"

#define DATA_PATH "../Lab2_child/data"


int main(int argc, char* argv[]) {
  if (argc < 3) {
    return 1;
  }
  CoffeeMachine coffeeMachine;
  if (QFile::exists(DATA_PATH + QString(argv[1]))) {
    coffeeMachine.setDescriptionPath(DATA_PATH + QString(argv[1]));
  } else {
    coffeeMachine.setDescriptionPath(DATA_PATH);
  }
  coffeeMachine.loadData();
  Human customer(atof(argv[2]));
  for (int i = 3; i < argc; i++) {
    coffeeMachine.sellProduct(argv[i], customer.giveMoney());
  }
  coffeeMachine.setDescriptionPath(DATA_PATH + QString(argv[1]));
  return 0;
}
