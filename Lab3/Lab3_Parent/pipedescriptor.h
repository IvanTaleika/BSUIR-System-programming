#ifndef PIPEDESCRIPTOR_H
#define PIPEDESCRIPTOR_H
#include <QString>
#include <unistd.h>
class PipeDescriptor {
 public:
  PipeDescriptor();
  ~PipeDescriptor();
  int init();
  int rDescriptor() const;
  char* rDescriptorStringValue() const;
  int closeRDescriptor();
  int wDescriptor() const;
  char* wDescriptorStringValue() const;
  int closeWDescriptor();

 private:
  int rDescriptor_;
  int wDescriptor_;
};

#endif // PIPEDESCRIPTOR_H
