#ifndef PRODUCT_H
#define PRODUCT_H
#include <QString>

class ProductInfo {

 public:
  ProductInfo(QString name, double price, unsigned int amount);
  QString toString() const;

  QString name() const;
  void setName(const QString& name);
  int amount() const;
  void setAmount(unsigned int amount);
  double price() const;
  void setPrice(double price);
  void sellOne();

 private:
  QString name_;
  unsigned int amount_;
  double price_;
};

#endif // PRODUCT_H
