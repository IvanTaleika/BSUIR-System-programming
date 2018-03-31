#include <QDir>
#include <QDebug>
#include <QList>
#include <QFile>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <unistd.h>
enum ERROR {
  NO_SOURCE_FILE_ERROR,
  SEMAPHORE_ERROR,
  FILE_ERROR
};

struct ThreadData {
  volatile sig_atomic_t* isThreadEnded;
  sem_t* semaphore;
  volatile QStringList* dataList;
  QFileInfoList* fileInfoList;
  ThreadData(volatile sig_atomic_t* isThreadEnded, sem_t* semaphore,
             volatile QStringList* dataList, QFileInfoList* fileInfoList) {
    this->isThreadEnded = isThreadEnded;
    this->semaphore = semaphore;
    this->dataList = dataList;
    this->fileInfoList = fileInfoList;
  }
};

void initData(QFileInfoList& fileInfoList, QDir fromDir, QFile& destination, QString to, sem_t* semaphore);
void showError(ERROR error);
void* threadReadFile(void* arg);
void threadCleanupHandler(void* arg);

int main(int argc, char* argv[]) {
  QDir sourceFileDir(QDir::current());
  sourceFileDir.cdUp();

  QFileInfoList fileInfoList;
  QFile destination;
  sem_t semaphore;

  initData(fileInfoList, sourceFileDir, destination,
           sourceFileDir.absolutePath() + "/Lab5/FileMerger.txt", &semaphore);

  volatile sig_atomic_t isThreadEnded = 0;
  pthread_t myThread;
  volatile QStringList dataList;

  ThreadData readerData(&isThreadEnded, &semaphore, &dataList, &fileInfoList);

  pthread_create(&myThread, NULL, threadReadFile, &readerData);
  int semaphoreValue;
  while (true) {
    sem_wait(&semaphore);
    sem_getvalue(&semaphore, &semaphoreValue);
    if (isThreadEnded && !semaphoreValue) {
      break;
    }
    destination.write(const_cast<QStringList&>(dataList).takeFirst().toUtf8());
  }
  destination.close();
  sem_destroy(&semaphore);
  return 0;
}

void initData(QFileInfoList& fileInfoList, QDir fromDir, QFile& destination, QString to, sem_t* semaphore) {
  fileInfoList = fromDir.entryInfoList(QStringList("*.txt"), QDir::Files);
  if (fileInfoList.isEmpty()) {
    showError(NO_SOURCE_FILE_ERROR);
    exit(NO_SOURCE_FILE_ERROR);
  }
  destination.setFileName(to);
  if (!destination.open(QIODevice::WriteOnly)) {
    showError(FILE_ERROR);
    exit(FILE_ERROR);
  }
  if (sem_init(semaphore, 0, 0) == -1) {
    showError(SEMAPHORE_ERROR);
    exit(SEMAPHORE_ERROR);
  }
}

void* threadReadFile(void* arg) {
  ThreadData* data = (ThreadData*)arg;
  pthread_cleanup_push(threadCleanupHandler, arg);
  if (data->fileInfoList->isEmpty()) {
    pthread_exit(nullptr);
  }

  for (int i = 0; i < data->fileInfoList->size(); i++) {
    QFile source(data->fileInfoList->at(i).absoluteFilePath());
    if (source.open(QIODevice::ReadOnly)) {
      while (!source.atEnd()) {
        const_cast<QStringList*>(data->dataList)->append(source.readLine());
        sem_post(data->semaphore);
      }
      source.close();
    } else {
      const_cast<QStringList*>(data->dataList)->append("Error with " + source.fileName());
      sem_post(data->semaphore);
    }
  }
  pthread_cleanup_pop(1);
  return nullptr;
}

void threadCleanupHandler(void* arg) {
  (*(((ThreadData*)arg)->isThreadEnded))++;
  sem_post(((ThreadData*)arg)->semaphore);
}

void showError(ERROR error) {
  QTextStream cout(stdout);
  switch (error) {
    case NO_SOURCE_FILE_ERROR:
      cout << "No source files" << endl;
      return;
    case SEMAPHORE_ERROR:
      cout << "Error with semaphore " << errno << endl;
      return;
    case FILE_ERROR:
      cout << "Connot open destination file" << endl;
      return;
  }
}


