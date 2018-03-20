#include "coffeemachine.h"


CoffeeMachine::CoffeeMachine() {
}

CoffeeMachine::CoffeeMachine(QString descriptionPath) : descriptionPath_(descriptionPath) {
  QFile description(descriptionPath_);
  if (description.exists() && description.open(QIODevice::ReadOnly)) {
    while (!description.atEnd()) {
      QString productDescripsion = description.readLine();
      QList<QString> productList = productDescripsion.split('\t');
      if (productList.size() == 3) {
        ProductInfo product(productList.at(0),
                            productList.at(1).toDouble(),
                            productList.at(2).toInt());
        productsInfo_.push_back(product);
      }
    }
    description.close();
  } else {
//    showError(STATE_ERROR);
  }

}

CoffeeMachine::~CoffeeMachine() {
  saveState();
}

void CoffeeMachine::ShowPriceList() {
  QTextStream cout(stdout);
  cout << "drink\tprice\tamount" << endl;
  for (int i = 0; i < productsInfo_.size(); i++) {
    cout << productsInfo_.at(i).toString() << endl;
  }
}

bool CoffeeMachine::sellProduct(QString productName, double& money) {
  for (int i = 0; i < productsInfo_.size(); i++) {
    if (productsInfo_.at(i).name() == productName) {
      if (productsInfo_.at(i).amount() > 0) {
        if (productsInfo_.at(i).price() <= money) {
          money -= productsInfo_.at(i).price();
          productsInfo_[i].sellOne();
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

void CoffeeMachine::saveState() {
  if (!descriptionPath_.isEmpty()) {
    QFile description(descriptionPath_);
    if (description.open(QIODevice::WriteOnly)) {
      for (int i = 0; i < productsInfo_.size(); i++) {
        description.write(productsInfo_.at(i).toString().toUtf8() + "\n");
      }
    }
    description.close();
  }
}

void CoffeeMachine::addProduct(QString name, double price, unsigned int amount) {
  ProductInfo product(name, price, amount);
  productsInfo_.push_back(product);
}


bool CoffeeMachine::loadData() {
  if (QFile::exists(descriptionPath_)) {
    QFile source(descriptionPath_);
    if (source.open(QIODevice::ReadOnly)) {
      productsInfo_.clear();
      while (!source.atEnd()) {
        QString productDescripsion = source.readLine();
        QList<QString> productList = productDescripsion.split('\t');
        if (productList.size() == 3) {
          ProductInfo product(productList.at(0),
                              productList.at(1).toDouble(),
                              productList.at(2).toInt());
          productsInfo_.push_back(product);
        }
      }
      source.close();
    }
  }
  return false;
}

QString CoffeeMachine::descriptionPath() const {
  return descriptionPath_;
}

void CoffeeMachine::setDescriptionPath(const QString& descriptionPath) {
  descriptionPath_ = descriptionPath;
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
