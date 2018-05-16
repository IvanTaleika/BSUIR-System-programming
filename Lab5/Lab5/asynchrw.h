#ifndef ASYNCHRW_H
#define ASYNCHRW_H
#include <QFile>
#include <QDir>
#include <QString>
#include <pthread.h>
#include <signal.h>
#include <QDebug>
#include <semaphore.h>
#include <functional>

class FileMerger {
 public:
  FileMerger(QFileInfoList sourceFilesInfo, QString destinationFileName,
             sem_t* mergeEndedSemaphore);
  ~FileMerger();
  int merge();
  void cancelMerging();
  bool isMerging();
 private:

  struct ThreadData {
    QFileInfoList sourceFilesInfo;
    QString destinationFileName;
    QStringList readList;

    sem_t* mergeEndedSemaphore;
    sem_t writeLockSemaphore;

    pthread_t writeThread;
    pthread_t readThread;

  };
  ThreadData threadData_;


  int createThread(void* (*startRoutine)(void*), pthread_t* thread);
  static void* readFiles(void* args);
  static void* writeFiles(void* args);
  static void readFilesEnd(void* args);
  static void writeFilesEnd(void* args);
};

#endif // ASYNCHRW_H
