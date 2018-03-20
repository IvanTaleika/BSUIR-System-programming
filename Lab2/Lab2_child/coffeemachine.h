#ifndef COFFEEMACHINE_H
#define COFFEEMACHINE_H
#include <QString>
#include <QFile>
#include <QVariant>
#include <QDebug>
#include <QTextStream>
#include "productinfo.h"

class CoffeeMachine {

 public:
  CoffeeMachine();
  CoffeeMachine(QString descriptionPath);
  ~CoffeeMachine();
  void ShowPriceList();
  bool sellProduct(QString productName, double& money);
  void saveState();
  void addProduct(QString name, double price, unsigned int amount);
  bool loadData();
  enum ERRORS {
    STATE_ERROR = 0,
    PRODUCT_ERROR = 1,
    AMOUNT_ERROR = 2,
    MONEY_ERROR = 3
  };
  QString descriptionPath() const;
  void setDescriptionPath(const QString& descriptionPath);

 private:
  QString descriptionPath_;
  QList<ProductInfo> productsInfo_;
  void showError(ERRORS errorCode);
};

#endif // COFFEEMACHINE_H
