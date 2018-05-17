#include "asynchrw.h"

FileMerger::FileMerger(QFileInfoList sourceFilesInfo, QString destinationFileName, sem_t* mergeEndedSemaphore) {
  threadData_.sourceFilesInfo = sourceFilesInfo;
  threadData_.destinationFileName = destinationFileName;
  threadData_.mergeEndedSemaphore = mergeEndedSemaphore;
  threadData_.readThread = 0;
  threadData_.writeThread = 0;
}

FileMerger::~FileMerger() {
  if (threadData_.readThread != 0) {
    pthread_cancel(threadData_.readThread);
  }
  if (threadData_.writeThread != 0) {
    pthread_cancel(threadData_.writeThread);
  }

}

int FileMerger::merge() {
  if (threadData_.readThread != 0 || threadData_.writeThread != 0) {
    return -1;
  }
  int error;
  threadData_.readList.clear();
  error = sem_init(&threadData_.writeLockSemaphore, 0, 0);
  if (error) {
    return error;
  }
  error = createThread(readFiles, &threadData_.readThread);
  if (error) {
    return error;
  }
  error = createThread(writeFiles, &threadData_.writeThread);
  return 0;
}

void FileMerger::cancelMerging() {
  if (threadData_.readThread != 0) {
    pthread_cancel(threadData_.readThread);
  }
  if (threadData_.writeThread != 0) {
    pthread_cancel(threadData_.writeThread);
  }
}

bool FileMerger::isMerging() {
  return threadData_.readThread != 0 || threadData_.writeThread != 0;
}

int FileMerger::createThread(void* (*startRoutine)(void*), pthread_t* thread) {
  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);
  return pthread_create(thread, &attribute, startRoutine, &threadData_);
}

void* FileMerger::readFiles(void* args) {
  pthread_cleanup_push_defer_np(readFilesEnd, args);
  ThreadData* data = (ThreadData*)args;
  QFile source;
  QString fileText;
  for (int i = 0; i < data->sourceFilesInfo.size(); i++) {
    source.setFileName(data->sourceFilesInfo.at(i).absoluteFilePath());
    if (source.open(QIODevice::ReadOnly)) {
      fileText = source.readAll();
      if (!fileText.isEmpty()) {
        data->readList.append(fileText);
        sem_post(&data->writeLockSemaphore);
      }
      source.close();
    }
  }
  data->readList.append("");
  sem_post(&data->writeLockSemaphore);
  pthread_cleanup_pop_restore_np(1);
  return nullptr;
}


void* FileMerger::writeFiles(void* args) {
  pthread_cleanup_push_defer_np(writeFilesEnd, args);
  ThreadData* data = (ThreadData*)args;
  QString fileText;
  QFile destination(data->destinationFileName);
  if (destination.open(QIODevice::WriteOnly)) {
    for (int i = 0;; i++) {
      sem_wait(&data->writeLockSemaphore);
      fileText = data->readList.at(i);
      if (fileText.isEmpty()) {
        break;
      }
      destination.write(fileText.toUtf8());
    }
    destination.close();
  } else {
    pthread_cancel(data->readThread);
    pthread_exit(nullptr);
  }
  pthread_cleanup_pop_restore_np(1);
  return nullptr;
}

void FileMerger::readFilesEnd(void* args) {
  ((ThreadData*)args)->readThread = 0;
}

void FileMerger::writeFilesEnd(void* args) {
  ThreadData* data = (ThreadData*)args;
  sem_destroy(&data->writeLockSemaphore);
  sem_post(data->mergeEndedSemaphore);
  data->writeThread = 0;
}

int merge(QFileInfoList sourceFilesInfo, QString destinationFileName, sem_t* mergeEndedSemaphore) {
  ThreadData* threadData = new ThreadData;
  threadData->sourceFilesInfo = sourceFilesInfo;
  threadData->destinationFileName = destinationFileName;
  threadData->mergeEndedSemaphore = mergeEndedSemaphore;
  threadData->readThread = 0;
  threadData->writeThread = 0;

  int error;
  error = sem_init(&threadData->writeLockSemaphore, 0, 0);
  if (error) {
    delete threadData;
    return error;
  }
  error = createThread(readFiles, &threadData->readThread, threadData);
  if (error) {
    return error;
    sem_destroy(&threadData->writeLockSemaphore);
    delete threadData;
    return error;
  }
  error = createThread(writeFiles, &threadData->writeThread, threadData);
  if (error) {
    pthread_cancel(threadData->readThread);
    sem_destroy(&threadData->writeLockSemaphore);
    delete threadData;
    return error;
  }
  return 0;
}

void* readFiles(void* args) {
  pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
  ThreadData* data = (ThreadData*)args;
  QFile source;
  QString fileText;
  for (int i = 0; i < data->sourceFilesInfo.size(); i++) {
    source.setFileName(data->sourceFilesInfo.at(i).absoluteFilePath());
    if (source.open(QIODevice::ReadOnly)) {
      fileText = source.readAll();
      if (!fileText.isEmpty()) {
        data->readList.append(fileText);
        sem_post(&data->writeLockSemaphore);
      }
      source.close();
    }
  }
  data->readList.append("");
  sem_post(&data->writeLockSemaphore);
  return nullptr;
}


void* writeFiles(void* args) {
  pthread_cleanup_push_defer_np(writeFilesEnd, args);
  ThreadData* data = (ThreadData*)args;
  QString fileText;
  QFile destination(data->destinationFileName);
  if (destination.open(QIODevice::WriteOnly)) {
    for (int i = 0;; i++) {
      sem_wait(&data->writeLockSemaphore);
      fileText = data->readList.at(i);
      if (fileText.isEmpty()) {
        break;
      }
      destination.write(fileText.toUtf8());
    }
    destination.close();
  } else {
    pthread_cancel(data->readThread);
    pthread_exit(nullptr);
  }
  pthread_cleanup_pop_restore_np(1);
  return nullptr;
}

void writeFilesEnd(void* args) {
  ThreadData* data = (ThreadData*)args;
  sem_destroy(&data->writeLockSemaphore);
  sem_post(data->mergeEndedSemaphore);
  delete data;
}

int createThread(void* (*startRoutine)(void*), pthread_t* thread, ThreadData* threadData) {
  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);
  return pthread_create(thread, &attribute, startRoutine, threadData);
}
