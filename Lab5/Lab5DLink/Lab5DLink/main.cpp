#include <QDir>
#include <QTextStream>
#include <QList>
#include <semaphore.h>
#include "asynchrw.h"

#define SOURCE_FILES_PATH "/home/ivan/Programing/Labs/SPO/Lab5/source"

enum ERROR {
  NO_SOURCE_FILES_ERROR,
  SEMAPHORE_ERROR,
};

sem_t mergeEndedSemaphore;

void initData(QFileInfoList& fileInfoList, QDir sourceFileDir);

int main(void) {
  QDir sourceFileDir(SOURCE_FILES_PATH);
  QFileInfoList fileInfoList;
  initData(fileInfoList, sourceFileDir);
  FileMerger fileMerger(fileInfoList, QDir::currentPath() + "/FileMerger.txt", &mergeEndedSemaphore);

  if (!fileMerger.merge()) {
    sem_wait(&mergeEndedSemaphore);
  } else {
    printf("Error!\n");
  }
  sem_destroy(&mergeEndedSemaphore);
  return 0;
}

void initData(QFileInfoList& fileInfoList, QDir sourceFileDir) {
  QTextStream cout(stdout);
  fileInfoList = sourceFileDir.entryInfoList(QStringList("*.txt"), QDir::Files);
  if (fileInfoList.isEmpty()) {
    cout << "No source files" << endl;
    exit(NO_SOURCE_FILES_ERROR);
  }

  if (sem_init(&mergeEndedSemaphore, 0, 0) == -1) {
    cout << "Error with semaphore: " << errno << endl;
    exit(SEMAPHORE_ERROR);
  }
}
