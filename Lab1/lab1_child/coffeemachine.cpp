#include "coffeemachine.h"


CoffeeMachine::CoffeeMachine(QString descriptionPath) {
  QFile description(descriptionPath);
  descriptionPath_ = descriptionPath;
  if (description.exists() && description.open(QIODevice::ReadOnly)) {

    while (!description.atEnd()) {
      QString productDescripsion = description.readLine();
      QList<QString> productList = productDescripsion.split('\t');
      if (productList.size() == 3) {
        productsInfo_.push_back(new ProductInfo(productList.at(0),
                                                productList.at(1).toDouble(),
                                                productList.at(2).toInt()));
      }
    }
    description.close();

  } else {
    showError(STATE_ERROR);
  }
}

CoffeeMachine::~CoffeeMachine() {
  QFile description(descriptionPath_);
  if (description.open(QIODevice::WriteOnly)) {
    for (int i = 0; i < productsInfo_.size(); i++) {
      description.write(productsInfo_.at(i)->toString().toUtf8() + "\n");
    }
    description.close();
  }
}

void CoffeeMachine::ShowPriceList() {
  QTextStream cout(stdout);
  cout << "drink\tprice\tamount" << endl;
  for (int i = 0; i < productsInfo_.size(); i++) {
    cout << productsInfo_.at(i)->toString() << endl;
  }
}

bool CoffeeMachine::sellProduct(QString productName, double& money) {
  for (int i = 0; i < productsInfo_.size(); i++) {
    if (productsInfo_.at(i)->name() == productName) {
      if (productsInfo_.at(i)->amount() > 0) {
        if (productsInfo_.at(i)->price() <= money) {
          money -= productsInfo_.at(i)->price();
          productsInfo_.at(i)->sellOne();
          //Return "Drink" object
          return true;
        }
//        showError(MONEY_ERROR);
        return false;
      }
//      showError(AMOUNT_ERROR);
      return false;
    }
  }
//  showError(PRODUCT_ERROR);
  return false;
}

void CoffeeMachine::showError(ERRORS errorCode) {
  QTextStream cout(stdout);
  switch (errorCode) {
    case STATE_ERROR:
      cout << "Machin is not working" << endl;
    case PRODUCT_ERROR:
      cout << "No such product." << endl;
      return;
    case AMOUNT_ERROR:
      cout << "Out of product." << endl;
      return;
    case MONEY_ERROR:
      cout << "Not enough money." << endl;
      break;
    default:
      return;
  }
}
