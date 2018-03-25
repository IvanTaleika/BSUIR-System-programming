#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <QList>
//for POSIX semophore
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include <csignal>
#include "pipedescriptor.h"
#define CHILD_PATH "/home/ivan/Programing/Labs/SPO/Lab3/build-Lab3_Child-Desktop_Qt_5_10_1_GCC_64bit-Debug/Lab3_Child"
#define DEBUG_PATH "/home/ivan/Programing/Labs/ConsoleSandbox/build-ConsoleSandbox-Desktop_Qt_5_10_1_GCC_64bit-Debug/ConsoleSandbox"

enum ERRORS {
  SIGNAL_HANDLER_ERROR,
  SEMAPHORE_ERROR,
  FORK_ERROR,
  EXEC_ERROR,
  PIPE_ERROR,
  SIGNAL_ERROR
};

QList<int> processesList;
sem_t* lab2Semaphore;
PipeDescriptor descriptor;

void showError(ERRORS errorIndex);
int setActionHandler();
void chldHandler(int iSignal);
pid_t createChild(int processIndex, PipeDescriptor& descriptor);
void freeResources();
int showPipeData();

int main(int argc, char* argv[]) {
  if (setActionHandler() == -1) {
    showError(SIGNAL_HANDLER_ERROR);
    return SIGNAL_HANDLER_ERROR;
  }
  lab2Semaphore = sem_open("/lab2Semaphore", O_CREAT, S_IRUSR | S_IWUSR, 1);
  if (lab2Semaphore == SEM_FAILED) {
    showError(SEMAPHORE_ERROR);
    return SEMAPHORE_ERROR;
  }
//  sem_post(lab2Semaphore);

  if (descriptor.init() == -1) {
    showError(PIPE_ERROR);
    return PIPE_ERROR;
  }
  fcntl(descriptor.rDescriptor(), F_SETFL, O_NONBLOCK);
  initscr();
  noecho();
  int childIndex = 0;
  while (true) {
    switch (getch()) {
      case '+':
        processesList.push_back(createChild(childIndex, descriptor));
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
        freeResources();
        kill(-getpid(), SIGTERM);
    }
  }
}

int setActionHandler() {
  struct sigaction chldStruct;
  chldStruct.sa_handler = chldHandler;
  chldStruct.sa_flags = SA_NOCLDSTOP | SA_RESTART; //do not resieve signal when close handle and process blocking signal
  return sigaction(SIGCHLD, &chldStruct, nullptr);
}

void chldHandler(int iSignal) {
  int index = processesList.indexOf(wait(0));
  if (index == -1) {
    showError(SIGNAL_ERROR);
    exit(SIGNAL_ERROR);
  }
  processesList.takeAt(index);
  showPipeData();
  sem_post(lab2Semaphore);
}

pid_t createChild(int processIndex, PipeDescriptor& descriptor) {
  pid_t pid = fork();
  switch (pid) {
    case -1:
      showError(FORK_ERROR);
      exit(FORK_ERROR);
    case 0: {
      int writeDescriptor = dup(descriptor.wDescriptor());
      descriptor.closeRDescriptor();
      descriptor.closeWDescriptor();
      if (execl(CHILD_PATH, CHILD_PATH, QString::number(processIndex).toUtf8().data(),
                QString::number(writeDescriptor).toUtf8().data(), NULL) == -1) {
        showError(EXEC_ERROR);
        exit(EXEC_ERROR);
      }
      return 0;
    }
    default:
      return pid;
  }
}

void showError(ERRORS errorIndex) {
  switch (errorIndex) {
    case SIGNAL_HANDLER_ERROR:
      printw("Error with sigaction: %s.\n", errno);
      break;
    case SEMAPHORE_ERROR:
      printw("Error with semaphore: %s.\n", errno);
      break;
    case FORK_ERROR:
      printw("Error with fork: %s.\n", errno);
      break;
    case EXEC_ERROR:
      printw("Error with exec: %s.\n", errno);
      break;
    case PIPE_ERROR:
      printw("Error with pipe: %s.\n", errno);
      break;
    case SIGNAL_ERROR:
      printw("Error with wait: %s.\n", errno);
      refresh();
      return;
  }
  printw(" Further work is impossible. Press any key to close application.\n");
  refresh();
  getch();
}

int showPipeData() {
  char buffer[100];
  int nBytes = read(descriptor.rDescriptor(), buffer, sizeof(buffer));
  if (nBytes == -1) {
    showError(PIPE_ERROR);
    exit(PIPE_ERROR);
  }
  printw("%s", buffer);
  refresh();
  return nBytes;
}

void freeResources() {
  endwin();
  sem_close(lab2Semaphore);
  sem_unlink("/lab2Semaphore");
  descriptor.closeRDescriptor();
  descriptor.closeWDescriptor();
  signal(SIGCHLD, SIG_DFL);
}
