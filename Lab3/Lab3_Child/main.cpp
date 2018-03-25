#include <sys/wait.h>
#include <unistd.h>
#include <QFile>
#include <QTextStream>

//for POSIX semophore
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <csignal>

#define FILE_PATH "../Lab3_Child/child_data"


int main(int argc, char* argv[]) {
  if (argc < 2) {
    return 0;
  }
  QTextStream cout(stdout);
  sem_t* lab2Semaphore = sem_open("/lab2Semaphore", O_CREAT, S_IRUSR | S_IWUSR, 1);
  if (lab2Semaphore == SEM_FAILED) {
    cout << "Error with opening semaphore." << endl;
    return 1;
  }

  QFile source(FILE_PATH);
  int stringNumber = atoi(argv[1]);
  if (!source.exists() || !source.open(QIODevice::ReadOnly)) {
    cout << "Error with source file opening." << endl;
    return 2;

  }
  for (int i = 0; i < stringNumber && !source.atEnd(); i++) {
    source.readLine();
  }
  QByteArray stringData;
  if (source.atEnd()) {
    stringData = QByteArray::number(stringNumber) + " End of source file reached.\n";
  } else {
    stringData = QByteArray::number(stringNumber) + " " + source.readLine();
  }
  source.close();
  int writeDescriptor = atoi(argv[2]);
  sem_wait(lab2Semaphore);
  if (write(writeDescriptor, stringData.data(), stringData.size() + 1) == -1) {
    cout << "Error with pipe." << endl;
    return 3;
  }
  sem_close(lab2Semaphore);
  close(writeDescriptor);

  return 0;
}
