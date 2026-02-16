#include <unistd.h>
#include <wordexp.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <readline/history.h>
#include <readline/readline.h>

char prompt[512];

void ignore_sigint(int sig) { write(STDOUT_FILENO, "\n", 1); write(STDOUT_FILENO, prompt, strlen(prompt)); }

int main() {
  printf("opsh: orca's Primitive SHell (Op-Shell) 1.3\n");
  char hostname[128];
  char wd[1024];
  char *path = getcwd(wd, 1023);
  gethostname(hostname, 127);
  snprintf(prompt, 512, "[\x1b[38;5;190m%s\x1b[0m@\x1b[38;5;190m%s\x1b[38;5;210m:\x1b[38;5;87m%s\x1b[0m]$ ", getenv("USER"), hostname, wd); // does not print to console
  char *command;
  for (;;) {
    signal(SIGINT, ignore_sigint); // ignore ^C in parent
    //char cmd[255];
    //printf(prompt); // prompt. customize as you want
    command = readline(prompt);
    if (!command) {
      printf("EOF reached\n");
      exit(0);
    }
    command[strcspn(command, "\n")] = '\0'; // null-terminate newlines when you enter the command
    if (*command == '\0') {
      continue;
    }
    if (command[0] == '#') {
      continue;
    }
    add_history(command); // so that arrow keys arrow key
    wordexp_t thing;
    if (wordexp(command, &thing, 0) != 0) {
      printf(command);
      printf("\n");
      //printf(thing);
      printf("opsh: Bad command or binary\n");
      continue;
    }
    if (strcmp(command, "exit") == 0) {
      printf("exit opsh\n");
      wordfree(&thing);
      exit(0);
    }
    if (strcmp(thing.we_wordv[0], "cd") == 0) {
      if (thing.we_wordv[1] != NULL) {
        if (chdir(thing.we_wordv[1]) != 0) {
          printf("opsh: failed to change directory\n");
          continue;
        }
        char buffer[1024];
        char *pathptr;
        pathptr = getcwd(buffer, 1023);
        if (pathptr != NULL) {
          printf("opsh: go to: %s\n", pathptr);
          wordfree(&thing);
        } else {
          printf("opsh: failure\n");
          wordfree(&thing);
          continue;
        }
      } else {
          char *pptr = getcwd(NULL, 0);
          if (!pptr) {
            printf("opsh: failure to display directory\n");
          } else {
            printf("opsh: you're in %s\n", pptr);
          }
          free(pptr); // otherwise memory leaks like crazy
      }
      wordfree(&thing);
      continue;
    }
    pid_t pid = fork();
    if (pid == 0) {
      if (execvp(thing.we_wordv[0],
        thing.we_wordv) == -1) {
        printf("opsh: Fail to launch command\n");
        wordfree(&thing);
        exit(1);
      }
      wordfree(&thing); // apparently necessary
    } else if (pid > 0) {
      signal(SIGINT, SIG_DFL);
      wait(NULL);
      wordfree(&thing);
    } else {
      printf("opsh: fork failed\n");
      wordfree(&thing);
      exit(1);
    }
  }
  return 0;
}
