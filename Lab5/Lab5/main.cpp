//#include <QDir>
//#include <QDebug>
//#include <QList>
//#include <semaphore.h>
//#include "asynchrw.h"

//enum ERROR {
//  NO_SOURCE_FILE_ERROR,
//  SEMAPHORE_ERROR,
//  FILE_ERROR,
//  SIGACTION_ERROR
//};


//sem_t readSemaphore;
//sem_t writeSemaphore;

//void initData(QFileInfoList& fileInfoList, QDir fromDir, QFile& destination, QString to);


//void showError(ERROR error);
//void readEndHandler(int iSignal);
//void writeEndHandler(int iSignal);

//int main(void) {
//  QDir sourceFileDir(QDir::current());
//  sourceFileDir.cdUp();

//  QFileInfoList fileInfoList;
//  QFile destination;
//  initData(fileInfoList, sourceFileDir, destination,
//           sourceFileDir.absolutePath() + "/Lab5/FileMerger.txt");

//  AsyncData readerData;
//  readerData.signal = SIGUSR1;
//  readerData.destinationThread = pthread_self();

//  AsyncData writerData;
//  writerData.signal = SIGUSR2;
//  writerData.file = &destination;
//  writerData.destinationThread = pthread_self();

//  QStringList dataList;
//  QFile file;
////  QFile file(fileInfoList.at(2).absoluteFilePath());
////  file.open(QIODevice::ReadOnly);
////  QString str = file.readAll();
////  qDebug() << str;
//  for (int i = 0; i < fileInfoList.size(); i++) {
//    file.setFileName(fileInfoList.at(i).absoluteFilePath());
//    if (file.open(QIODevice::ReadOnly)) {
//      qDebug() << i;
//      readerData.file = &file;
//      dataList.append("");
//      readerData.string = &dataList[i];
//      asynchRead(&readerData);

//      sem_wait(&readSemaphore);
//      sem_wait(&writeSemaphore);

//      writerData.string = &dataList[i];
//      asynchWrite(&writerData);

//      file.close();
//    }
//  }
//  qDebug() << "we are here";
//  sem_wait(&writeSemaphore);
//  qDebug() << "and here";
//  sem_destroy(&writeSemaphore);
//  sem_destroy(&readSemaphore);
//  destination.close();
//  return 0;
//}

//void initData(QFileInfoList& fileInfoList, QDir fromDir,
//              QFile& destination, QString to) {
//  fileInfoList = fromDir.entryInfoList(QStringList("*.txt"), QDir::Files);
//  if (fileInfoList.isEmpty()) {
//    showError(NO_SOURCE_FILE_ERROR);
//    exit(NO_SOURCE_FILE_ERROR);
//  }
//  destination.setFileName(to);
//  if (!destination.open(QIODevice::WriteOnly)) {
//    showError(FILE_ERROR);
//    exit(FILE_ERROR);
//  }
//  if (sem_init(&readSemaphore, 0, 0) == -1) {
//    showError(SEMAPHORE_ERROR);
//    exit(SEMAPHORE_ERROR);
//  }
//  struct sigaction readStruct;
//  readStruct.sa_handler = readEndHandler;
//  if (sigaction(SIGUSR1, &readStruct, NULL) == -1) {
//    showError(SIGACTION_ERROR);
//    exit(SIGACTION_ERROR);
//  }
//  if (sem_init(&writeSemaphore, 0, 1) == -1) {
//    showError(SEMAPHORE_ERROR);
//    exit(SEMAPHORE_ERROR);
//  }
//  struct sigaction writeStruct;
//  writeStruct.sa_handler = writeEndHandler;
//  if (sigaction(SIGUSR2, &writeStruct, NULL) == -1) {
//    showError(SIGACTION_ERROR);
//    exit(SIGACTION_ERROR);
//  }
//}

//void showError(ERROR error) {
//  QTextStream cout(stdout);
//  switch (error) {
//    case NO_SOURCE_FILE_ERROR:
//      cout << "No source files" << endl;
//      return;
//    case SEMAPHORE_ERROR:
//      cout << "Error with semaphore: " << errno << endl;
//      return;
//    case FILE_ERROR:
//      cout << "Connot open destination file" << endl;
//      return;
//    case SIGACTION_ERROR:
//      cout << "Error with sigaction: " << errno << endl;
//      return;
//  }
//}

//void writeEndHandler(int iSignal) {
//  sem_post(&writeSemaphore);
//}

//void readEndHandler(int iSignal) {
//  sem_post(&readSemaphore);
//}

#include <QDir>
#include <QDebug>
#include <QList>
#include <semaphore.h>
#include "asynchrw.h"

enum ERROR {
  NO_SOURCE_FILE_ERROR,
  SEMAPHORE_ERROR,
  FILE_ERROR,
  SIGACTION_ERROR
};

sem_t mergeEndedSemaphore;

void initData(QFileInfoList& fileInfoList, QDir sourceFileDir);


void showError(ERROR error);


int main(void) {
  QDir sourceFileDir(QDir::current());
  sourceFileDir.cdUp();
  QFileInfoList fileInfoList;
  initData(fileInfoList, sourceFileDir);
  FileMerger fileMerger(fileInfoList, sourceFileDir.absolutePath() + "/Lab5/FileMerger.txt", &mergeEndedSemaphore);

  if (!fileMerger.merge()) {
    sem_wait(&mergeEndedSemaphore);
  } else {
    printf("Error!\n");
  }
  sem_destroy(&mergeEndedSemaphore);
  return 0;
}

void initData(QFileInfoList& fileInfoList, QDir sourceFileDir) {
  fileInfoList = sourceFileDir.entryInfoList(QStringList("*.txt"), QDir::Files);
  if (fileInfoList.isEmpty()) {
    showError(NO_SOURCE_FILE_ERROR);
    exit(NO_SOURCE_FILE_ERROR);
  }

  if (sem_init(&mergeEndedSemaphore, 0, 0) == -1) {
    showError(SEMAPHORE_ERROR);
    exit(SEMAPHORE_ERROR);
  }
}

void showError(ERROR error) {
  QTextStream cout(stdout);
  switch (error) {
    case NO_SOURCE_FILE_ERROR:
      cout << "No source files" << endl;
      return;
    case SEMAPHORE_ERROR:
      cout << "Error with semaphore: " << errno << endl;
      return;
    case FILE_ERROR:
      cout << "Connot open destination file" << endl;
      return;
    case SIGACTION_ERROR:
      cout << "Error with sigaction: " << errno << endl;
      return;
  }
}
