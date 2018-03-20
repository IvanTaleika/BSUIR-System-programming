#include "human.h"

QString Human::getProductName() {
  QTextStream cout(stdout);
  QTextStream cin(stdin);
  cout << "Enter product name" << endl;
  QString name;
  name = cin.readLine();
  return name;
}

Human::Human(const double& money) {
  money_ = money;
}

void Human::buyProduct(CoffeeMachine& coffeeMachine) {
  coffeeMachine.ShowPriceList();
  showMoney();
  coffeeMachine.sellProduct(getProductName(), money_);
}

void Human::showMoney() {
  QTextStream cout(stdout);
  cout << "You have " << money_ << " RUB" << endl;
}

double& Human::giveMoney() {
  return money_;
}

double Human::money() const {
  return money_;
}

void Human::setMoney(double money) {
  money_ = money;
}

