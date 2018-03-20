#include <sys/wait.h>
#include <unistd.h>
#include <QFile>
#include <QTextStream>

//for POSIX semophore
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <csignal>

#define CHILD_PATH "/home/ivan/Programing/Labs/SPO/Lab2/build-Lab2_child-Desktop_Qt_5_10_1_GCC_64bit-Debug/Lab2_child"
#define FILE_PATH "../Lab2_parent/parent_data"

int main(int argc, char* argv[]) {
  if (argc < 1) {
    return 0;
  }
  QTextStream cout(stdout);
  sem_t* lab2Semaphore = sem_open("/lab2Semaphore", O_CREAT, S_IRUSR | S_IWUSR, 1);
  if (lab2Semaphore == SEM_FAILED) {
    cout << "Error with opening semaphore." << endl;
    return 1;
  }
  pid_t pid = fork();

  switch (pid) {
    case -1: {
      cout << "Error with fork: " << errno << endl;
      break;
    }
    case 0: {
      if (execlp(CHILD_PATH, CHILD_PATH, argv[1], "6", "Americano", "Latte Macchiato", "Capuchino", "Americano", NULL) == -1) {
        cout << "Error with exec: " << errno << endl;
        exit(1);
      }
      break;
    }
    default: {
      QFile source(FILE_PATH);
      int stringNumber = atoi(argv[1]);
      if (!source.exists() || !source.open(QIODevice::ReadOnly)) {
        cout << "Error with source file." << endl;
        break;
      }
      for (int i = 0; i < stringNumber; i++) {
        source.readLine();
      }
      char symbol;
      sem_wait(lab2Semaphore);
      while (source.getChar(&symbol) && symbol != '\n') {
        cout << symbol;
        cout.flush();
        usleep(100000);
      }
      cout << endl;
    }
  }
  sem_post(lab2Semaphore);
  sem_close(lab2Semaphore);
  wait(0);

  return 0;
}
