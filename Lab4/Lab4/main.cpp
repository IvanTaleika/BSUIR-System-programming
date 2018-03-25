#include <pthread.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <unistd.h>
#include <ncurses.h>
#include <QList>
#include <csignal>

enum ERRORS {
  NO_COMMAND_LINE_ARGUMENTS,
  SIGNAL_HANDLER_ERROR,
  MUTEX_ERROR,
  TIMER_ERROR
};

QList<pthread_t> threadList;
int threadTidOutputTimer;
pthread_mutex_t mutex;

void* threadFunction(void* arg);
void setupResources(int argc, char* argv[]);
void showError(ERRORS errorIndex);
int setActionHandler();
int setTimer(time_t secExpireTime);
void createNewThread(int iSignal);
void freeResources();

int main(int argc, char* argv[]) {
  setupResources(argc, argv);
  while (true) {
    switch (getch()) {
      case '+':
        createNewThread(SIGVTALRM);
        break;
      case '-':
        if (!threadList.isEmpty()) {
          pthread_cancel(threadList.takeLast());
        }
        break;
      case 'q':
        freeResources();
        return 0; //Equal to exit_group
    }
  }
}

void setupResources(int argc, char* argv[]) {
  initscr();
  noecho();
  scrollok(stdscr, true);

  if (argc < 3) {
    showError(NO_COMMAND_LINE_ARGUMENTS);
    exit(NO_COMMAND_LINE_ARGUMENTS);
  }

  if (setActionHandler() == -1) {
    showError(SIGNAL_HANDLER_ERROR);
    exit(SIGNAL_HANDLER_ERROR);
  }

  if (pthread_mutex_init(&mutex, NULL) != 0) {
    showError(MUTEX_ERROR);
    exit(MUTEX_ERROR);
  }

  threadTidOutputTimer = atoi(argv[1]);

  if (setTimer(atoi(argv[2])) == -1) {
    showError(TIMER_ERROR);
    exit(TIMER_ERROR);
  }
}

int setActionHandler() {
  struct sigaction alrmStruct;
  alrmStruct.sa_handler = createNewThread;
  return sigaction(SIGALRM, &alrmStruct, nullptr);
}

void createNewThread(int iSignal) {
  pthread_t tid;
  pthread_attr_t attribute;
  pthread_attr_init(&attribute);
  pthread_attr_setdetachstate(&attribute, PTHREAD_CREATE_DETACHED);
  pthread_create(&tid, &attribute, threadFunction, &threadTidOutputTimer);
  threadList.append(tid);
}

int setTimer(time_t secExpireTime) {
  itimerval timerValue;
  timerValue.it_interval.tv_usec = 0;
  timerValue.it_interval.tv_sec = secExpireTime;
  timerValue.it_value.tv_usec = 0;
  timerValue.it_value.tv_sec = secExpireTime;
  return setitimer(ITIMER_REAL, &timerValue, NULL);
}

void* threadFunction(void* arg) {
  int sleepTime = *(int*)arg;
  pthread_t tid = pthread_self();
  while (true) {
    pthread_testcancel();
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock(&mutex);
    printw("Thread id: %d\n", tid);
    refresh();
    pthread_mutex_unlock(&mutex);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    sleep(sleepTime);
  }
}

void showError(ERRORS errorIndex) {
  switch (errorIndex) {
    case NO_COMMAND_LINE_ARGUMENTS:
      printw("Programm requires 2 comand line arguments: 1st is a time between thread info output "
             "and second is a time between auto creating a new thread (seconds).\n");
      break;
    case SIGNAL_HANDLER_ERROR:
      printw("Error with sigaction: %s.\n", errno);
      break;
    case MUTEX_ERROR:
      printw("Error with mutex: %s.\n", errno);
      break;
    case TIMER_ERROR:
      printw("Error with setitimer: %s.\n", errno);
      break;
  }
  printw(" Further work is impossible. Press any key to close application.\n");
  refresh();
  getch();
}

void freeResources() {
  endwin();
  if (!pthread_mutex_trylock(&mutex)) {
    pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
  }
  signal(SIGALRM, SIG_DFL);
}
