#ifndef ASYNCHRW_H
#define ASYNCHRW_H
#include <QFile>
#include <QString>
#include <pthread.h>
#include <signal.h>

#include "asynchrw_global.h"

struct ASYNCHRWSHARED_EXPORT AsyncData {
  int signal;
  QFile* file;
  QString* string;
};

ASYNCHRWSHARED_EXPORT int  asynchRead(AsyncData* asyncData);
void*  readThreadStart(void* arg);
ASYNCHRWSHARED_EXPORT int asynchWrite(AsyncData* asyncData);
void*  writeThreadStart(void* arg);

#endif // ASYNCHRW_H
