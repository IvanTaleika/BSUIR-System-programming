#include "coffeemachine.h"
#include "human.h"


int main(int argc, char* argv[]) {
  CoffeeMachine coffeeMachine("../lab1_child/data");
  Human customer(atof(argv[1]));
  for (int i = 2; i < argc; i++) {
    coffeeMachine.sellProduct(argv[i], customer.giveMoney());

  }
  QTextStream cin(stdin);
  cin.read(1);

//  QString buyMore;
//  QTextStream cout(stdout);
//  do {
//    QProcess::execute("clear");
//    customer.buyProduct(coffeeMachine);
//    cout << "Buy more? (enter 0 to exit)" << endl;
//    buyMore = cin.readLine();
//  } while (buyMore.compare("0"));
  return 0;
}
