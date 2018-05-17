#ifndef ASYNCHRW_H
#define ASYNCHRW_H
#include <QFile>
#include <QDir>
#include <QString>
#include <pthread.h>
#include <semaphore.h>
#include "asynchrw_global.h"


struct ThreadData {
  QFileInfoList sourceFilesInfo;
  QString destinationFileName;
  QStringList readList;

  sem_t* mergeEndedSemaphore;
  sem_t writeLockSemaphore;

  pthread_t writeThread;
  pthread_t readThread;
};

class ASYNCHRWSHARED_EXPORT FileMerger {
 public:
  FileMerger(QFileInfoList sourceFilesInfo, QString destinationFileName,
             sem_t* mergeEndedSemaphore);
  virtual ~FileMerger();
  int merge();
  void cancelMerging();
  bool isMerging();
 private:

  ThreadData threadData_;


  int createThread(void* (*startRoutine)(void*), pthread_t* thread);
  static void* readFiles(void* args);
  static void* writeFiles(void* args);
  static void readFilesEnd(void* args);
  static void writeFilesEnd(void* args);
};

extern "C" ASYNCHRWSHARED_EXPORT
int merge(QFileInfoList sourceFilesInfo, QString destinationFileName, sem_t* mergeEndedSemaphore);
extern "C" int createThread(void* (*startRoutine)(void*), pthread_t* thread, ThreadData* threadData);
extern "C" void* readFiles(void* args);
extern "C" void* writeFiles(void* args);
extern "C" void writeFilesEnd(void* args);

#endif // ASYNCHRW_H
