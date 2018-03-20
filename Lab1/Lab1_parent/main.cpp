#include <sys/wait.h>
#include <unistd.h>
#include <ncurses.h>
#include <ctime>

#define DEBUG_PATH "/home/ivan/Programing/Labs/ConsoleSandbox/build-ConsoleSandbox-Desktop_Qt_5_10_1_GCC_64bit-Debug/ConsoleSandbox"
#define CHILD_PATH "/home/ivan/Programing/Labs/SPO/Lab1/build-lab1_child-Desktop_Qt_5_10_1_GCC_64bit-Debug/lab1_child"

int main() {

  initscr();
  curs_set(0);

  pid_t pid = fork();

  switch (pid) {
    case -1: {
      printw("Error with creating child process.");
      break;
    }

    case 0: {
      if (execlp(CHILD_PATH, "", "6", "Americano", "Latte Macchiato", "Capichino", "Americano", NULL) == -1) {
        printw("Error with child file finding.");
      }
      break;
    }

    default: {
      int st;
      time_t ltime;

      while (1) {
        refresh();
        time(&ltime);
        move(1, 0);
        printw(ctime(&ltime));

        if (waitpid(pid, &st, WNOHANG) > 0) {
          break;
        }

        napms(50);
      }
      printw("The child process was terminated. Status: ");
      if (WIFEXITED(st)) {
        printw("%d", (int)WEXITSTATUS(st));
      }
    }
  }
  getch();
  endwin();
  return 0;
}
