#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <QList>
//for POSIX semophore
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <csignal>

#define CHILD_PATH "/home/ivan/Programing/Labs/SPO/Lab2/build-Lab2_parent-Desktop_Qt_5_10_1_GCC_64bit-Debug/Lab2_parent"

enum ERRORS {
  SIGNAL_HANDLER_ERROR,
  SEMAPHORE_ERROR,
  SIGNAL_ERROR
};

QList<int> processesList;
sem_t* lab2Semaphore;

void showError(ERRORS errorIndex);
int setActionHandler();
void chldHandler(int iSignal);
pid_t createChild(int processIndex);

int main(int argc, char* argv[]) {
  if (setActionHandler() == -1) {
    showError(SIGNAL_HANDLER_ERROR);
    return 1;
  }

  lab2Semaphore = sem_open("/lab2Semaphore", O_CREAT, S_IRUSR | S_IWUSR, 1);
  if (lab2Semaphore == SEM_FAILED) {
    showError(SEMAPHORE_ERROR);
    return 1;
  }
  initscr();
  noecho();

  int childIndex = 0;

  while (true) {
    switch (getch()) {
      case '+':
        processesList.push_back(createChild(childIndex));
        childIndex++;
        break;
      case '-':
        if (!processesList.isEmpty()) {
          kill(processesList.last(), SIGTERM);
        }
        if (childIndex > 0) {
          childIndex--;
        }
        break;
      case 'q':
        endwin();
        sem_close(lab2Semaphore);
        sem_unlink("/lab2Semaphore");
        signal(SIGCHLD, SIG_DFL);
        kill(-getpid(), SIGTERM);
    }
  }
}

int setActionHandler() {
  struct sigaction chldStruct;
  chldStruct.sa_handler = chldHandler;
  return sigaction(SIGCHLD, &chldStruct, nullptr);
}

void chldHandler(int iSignal) {
  int index = processesList.indexOf(wait(0));
  if (index == -1) {
    showError(SIGNAL_ERROR);
    return;
  }
  processesList.takeAt(index);
  int value;
  sem_getvalue(lab2Semaphore, &value);
  if (processesList.isEmpty() && !value) {
    sem_post(lab2Semaphore);
  }

}

pid_t createChild(int processIndex) {
  pid_t pid = fork();
  switch (pid) {
    case -1:
      exit(1);
    case 0:
      if (execl(CHILD_PATH, CHILD_PATH, QString::number(processIndex).toUtf8().data(), NULL) == -1) {
        exit(2);
      }
      return 0;
    default:
      return pid;
  }
}

void showError(ERRORS errorIndex) {
  switch (errorIndex) {
    case SIGNAL_HANDLER_ERROR:
      printw("Error with setting action handler. Further work is impossible.\n");
      break;
    case SEMAPHORE_ERROR:
      printw("Error with creating semaphore. Further work is impossible.\n");
      break;
    case SIGNAL_ERROR:
      printw("Unknown signal resieved. Further work can be unsafe.\n");
      break;
  }
  printw("Press any key.\n");
  refresh();
  getch();
}

