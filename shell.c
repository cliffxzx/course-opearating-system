#include "ctype.h"
#include "errno.h"
#include "fcntl.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "sys/stat.h"
#include "sys/types.h"
#include "sys/wait.h"
#include "unistd.h"

#define MAX_LINE 10240
#define MAX_BUFFER 20480

#define RUN 0b01
#define STOP 0b10
int status = RUN;

int redirect_out(char *name) {
  int fd = open(name, O_WRONLY | O_CREAT | O_TRUNC, 0777);
  dup2(fd, 1);
  return close(fd);
}

int redirect_in(char *name) {
  int fd = open(name, O_RDONLY, 0777);
  dup2(fd, 0);
  return close(fd);
}

void exec_command(char *args[]) {
  if (!strcmp(args[0], "exit")) {
    status = STOP;
    return;
  } else if (!strcmp(args[0], "cd")) {
    chdir(args[1]);
    return;
  }

  pid_t pid = fork();
  if (pid < 0) {
    printf("Error!\n");
    exit(126);
  } else if (!pid)
    execvp(args[0], args);
  else if (pid > 0)
    waitpid(pid, NULL, 0);
}

int exec_command_pipe(char *args[]) {
  int fd[2];
  pipe(fd);

  dup2(fd[1], 1);
  close(fd[1]);

  exec_command(args);

  dup2(fd[0], 0);
  return close(fd[0]) + 1;
}

void tokenize(char *dest, char *str) {
  int i = 0;
  for (int w = 0; str[w]; ++w)
    if (str[w] == '>' || str[w] == '|' || str[w] == '<') {
      dest[i++] = ' ';
      dest[i++] = str[w];
      dest[i++] = ' ';
    } else
      dest[i++] = str[w];

  dest[i - 1] = '\0';
}

int main() {
  int stdio_temp[] = {dup(0), dup(1)};

  while (status & RUN) {
    fflush(stdout);
    dup2(stdio_temp[1], 1);
    printf("%s $ ", getcwd(NULL, NULL));

    char line[MAX_LINE];
    fgets(line, MAX_LINE, stdin);

    char tokenized[MAX_BUFFER];
    tokenize(tokenized, line);

    int i = 0;
    char *args[MAX_LINE];

    int reset_stdout = 0;

    char *arg = strtok(tokenized, " ");
    while (arg) {
      if (arg[0] == '>')
        reset_stdout = redirect_out(strtok(NULL, " "));
      else if (arg[0] == '<')
        reset_stdout = redirect_in(strtok(NULL, " "));
      else if (arg[0] == '|') {
        args[i++] = NULL;
        reset_stdout = exec_command_pipe(args);
        i = 0;
      } else
        args[i++] = arg;

      arg = strtok(NULL, " ");
    }

    args[i++] = NULL;

    if (reset_stdout)
      dup2(stdio_temp[1], 1);

    exec_command(args);

    dup2(stdio_temp[0], 0);
  }

  return 0;
}