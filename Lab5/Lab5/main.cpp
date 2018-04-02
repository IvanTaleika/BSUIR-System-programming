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


sem_t semaphore;

void initData(QFileInfoList& fileInfoList, QDir fromDir, QFile& destination, QString to, sem_t* semaphore);


void showError(ERROR error);
void readEndHandler(int iSignal);

int main(void) {
  QDir sourceFileDir(QDir::current());
  sourceFileDir.cdUp();

  QFileInfoList fileInfoList;
  QFile destination;

  initData(fileInfoList, sourceFileDir, destination,
           sourceFileDir.absolutePath() + "/Lab5/FileMerger.txt", &semaphore);

  QVector<AsyncData> readerDataVector(fileInfoList.size());
  QStringList dataList;

  for (int i = 0; i < readerDataVector.size(); i++) {
    QFile* file = new QFile(fileInfoList.at(i).absoluteFilePath());
    if (file->open(QIODevice::ReadOnly)) {
      readerDataVector[i].file = file;
      readerDataVector[i].signal = SIGUSR1;
      dataList.append("");
      readerDataVector[i].string = &dataList[i];
      asynchRead(&readerDataVector[i]);
    } else {
      delete file;
      readerDataVector.removeAt(i);
      i--;
    }
  }
  QVector<AsyncData> writerDataVector(readerDataVector.size());

  for (int i = 0; i < writerDataVector.size(); i++) {
    writerDataVector[i].signal = -1;
    writerDataVector[i].file = &destination;
    writerDataVector[i].string = &dataList[i];
    sem_wait(&semaphore);
    asynchWrite(&writerDataVector[i]);

  }
  sem_destroy(&semaphore);
  for (int i = 0; i < readerDataVector.size(); i++) {
    readerDataVector[i].file->close();
    delete readerDataVector[i].file;
  }

  destination.close();
  return 0;
}

void initData(QFileInfoList& fileInfoList, QDir fromDir,
              QFile& destination, QString to, sem_t* semaphore) {
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
  struct sigaction readStruct;
  readStruct.sa_handler = readEndHandler;
  if (sigaction(SIGUSR1, &readStruct, NULL) == -1) {
    showError(SIGACTION_ERROR);
    exit(SIGACTION_ERROR);
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


void readEndHandler(int iSignal) {
  sem_post(&semaphore);
}
