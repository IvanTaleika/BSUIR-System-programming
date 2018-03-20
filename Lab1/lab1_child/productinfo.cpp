#include "productinfo.h"
#include <QTextStream>

QString ProductInfo::name() const {
  return name_;
}

void ProductInfo::setName(const QString& name) {
  name_ = name;
}

int ProductInfo::amount() const {
  return amount_;
}

void ProductInfo::setAmount(int amount) {
  amount_ = amount;
}

double ProductInfo::price() const {
  return price_;
}

void ProductInfo::setPrice(double price) {
  price_ = price;
}

void ProductInfo::sellOne() {
  amount_--;
}

ProductInfo::ProductInfo(QString name, double price, int amount) {
  name_ = name;
  amount_ = amount;
  price_ = price;
}

QString ProductInfo::toString() {
  return name_ + "\t" + QString::number(price_) + "\t" + QString::number(amount_);
}


