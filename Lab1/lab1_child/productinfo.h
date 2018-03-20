#ifndef PRODUCT_H
#define PRODUCT_H
#include <QString>

class ProductInfo {

 public:
  ProductInfo(QString name, double price, int amount);
  QString toString();

  QString name() const;
  void setName(const QString& name);
  int amount() const;
  void setAmount(int amount);
  double price() const;
  void setPrice(double price);
  void sellOne();

 private:
  QString name_;
  int amount_;
  double price_;
};

#endif // PRODUCT_H
