#include "stdio.h"
#include "stdlib.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "unistd.h"
#include <errno.h>

int main() {
  pid_t pid = fork();
  if (pid < 0) {
    printf("Error!");
    exit(126);
  } else if (!pid) {
    // 1. child and parent pid.
    printf("This is child process, pid is: %d, pid of parent is: %d\n", getpid(), getppid());

    printf("what's your name?\n");
    char name[126];
    scanf("%s", name);
    printf("You input: \"%s\" \n", name);
    exit(0);
  } else if (pid > 0) {
    // 1. parent and child pid.
    printf("This is parent process, pid is %d, pid of child is: %d\n", getpid(), pid);

    // 2. control the execute order, the parent process will wait child process.
    int status_code;
    waitpid(pid, &status_code, 0);
    printf("The parent process wait child execute done.\n");
  }

  return 0;
}