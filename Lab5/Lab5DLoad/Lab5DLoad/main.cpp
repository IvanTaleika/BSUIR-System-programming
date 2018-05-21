#include <QDir>
#include <QTextStream>
#include <QList>
#include <semaphore.h>
#include <QLibrary>

#define SHARED_LIBRARY_PATH "/home/ivan/Programing/Labs/SPO/Lab5/lib/libasynchRW.so"
#define FUNCTION_NAME "merge"

enum ERROR {
  NO_SOURCE_FILES_ERROR,
  SEMAPHORE_ERROR,
  LIBRARY_ERROR,
};

typedef int (*FileMergerFunction)(QFileInfoList, QString, sem_t*);
sem_t mergeEndedSemaphore;

void initData(QFileInfoList& fileInfoList, QDir sourceFileDir, FileMergerFunction& mergeFunction);


int main(void) {
  QDir sourceFileDir("/home/ivan/Programing/Labs/SPO/Lab5/source");
  QFileInfoList fileInfoList;
  FileMergerFunction mergeFunction;
  initData(fileInfoList, sourceFileDir, mergeFunction);

  int error = mergeFunction(fileInfoList, QDir::currentPath() + "/FileMerger.txt", &mergeEndedSemaphore);
  if (!error) {
    sem_wait(&mergeEndedSemaphore);
  } else {
    printf("Error!\n");
  }
  sem_destroy(&mergeEndedSemaphore);
  return 0;
}

void initData(QFileInfoList& fileInfoList, QDir sourceFileDir, FileMergerFunction& mergeFunction) {
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

  QLibrary asynchRWLib(SHARED_LIBRARY_PATH);
  mergeFunction = (FileMergerFunction) asynchRWLib.resolve(FUNCTION_NAME);
  if (mergeFunction == nullptr) {
    cout << "The symbol could not be resolved or if the library could not be loaded" << endl;
    exit(LIBRARY_ERROR);
  }
}
