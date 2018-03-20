#ifndef HUMAN_H
#define HUMAN_H
#include "coffeemachine.h"
#include "QTextStream"
class Human {

 public:
  Human(const double& money);
  void buyProduct(CoffeeMachine& coffeeMachine);
  void showMoney();
  double& giveMoney();
  double money() const;
  void setMoney(double money);

 private:
  double money_;
  QString getProductName();

};

#endif // HUMAN_H
