#include "pipedescriptor.h"

PipeDescriptor::PipeDescriptor() : rDescriptor_(-1), wDescriptor_(-1) {
}

PipeDescriptor::~PipeDescriptor() {
  if (rDescriptor_ != -1) {
    close(rDescriptor_);
  }
  if (wDescriptor_ != -1) {
    close(wDescriptor_);
  }
}

int PipeDescriptor::init() {
  int tempDescriptors[2];
  if (pipe(tempDescriptors) == -1) {
    rDescriptor_ = -1;
    wDescriptor_ = -1;
    return -1;
  }
  rDescriptor_ = tempDescriptors[0];
  wDescriptor_ = tempDescriptors[1];
  return 0;
}

int PipeDescriptor::rDescriptor() const {
  return rDescriptor_;
}

char* PipeDescriptor::rDescriptorStringValue() const {
  return QString::number(wDescriptor_).toUtf8().data();
}

int PipeDescriptor::closeRDescriptor() {
  if (rDescriptor_ == -1) {
    return -1;
  }
  if (close(rDescriptor_) == -1) {
    return -1;
  } else {
    rDescriptor_ = -1;
    return 0;
  }
}

int PipeDescriptor::wDescriptor() const {
  return wDescriptor_;
}

char* PipeDescriptor::wDescriptorStringValue() const {
  return QString::number(wDescriptor_).toUtf8().data();
}

int PipeDescriptor::closeWDescriptor() {
  if (wDescriptor_ == -1) {
    return -1;
  }
  if (close(wDescriptor_) == -1) {
    return -1;
  } else {
    wDescriptor_ = -1;
    return 0;
  }
}

