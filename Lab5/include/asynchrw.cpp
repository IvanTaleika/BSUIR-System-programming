#include "asynchrw.h"

int asynchRead(AsyncData* asyncData) {
  pthread_t thread;

  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);

  return pthread_create(&thread, NULL, readThreadStart, asyncData);
}

void* readThreadStart(void* arg) {

  AsyncData* data = (AsyncData*)arg;
  *(data->string) += data->file->readAll();
  if (data->signal >= 0) {
    raise(data->signal);
  }
  return nullptr;
}

int asynchWrite(AsyncData* asyncData) {
  pthread_t thread;

  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);

  return pthread_create(&thread, &attribute, writeThreadStart, asyncData);
}

void* writeThreadStart(void* arg) {
  AsyncData* data = (AsyncData*)arg;
  data->file->write(data->string->toUtf8());
  if (data->signal >= 0) {
    raise(data->signal);
  }
  return nullptr;
}
