#include <unistd.h>
#include <wordexp.h>
#include <stdio.h>
#include <sys/wait.h> // so that the parent waits for the child
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "opsh.h"

int main(int argc, char **argv) {
  (void)argc;
  if (argv[1]) {
    if (!strcmp(argv[1], "--cat")) {
      printf("meow\n"); // we need humor
      exit(0);
    }
  }
  signal(SIGINT, ignore_sigint);
  signal(SIGHUP, sighup_notify);
  signal(SIGQUIT, sigquit_notify);
  signal(SIGILL, sigill_notify);
  signal(SIGSEGV, sigsegv_notify);
  printf("opsh: orca's Primitive SHell (Op-Shell) 1.7\n");
  //write_prompt(); // does not print to console
  char *command;
  for (;;) {
    //signal(SIGINT, ignore_sigint); // ignore ^C in parent
    char cmd[255];
    printf(write_prompt()); // prompt. customize as you want
    command = fgets(cmd, sizeof(cmd), stdin);
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
    //add_history(command); // so that arrow keys arrow key
    wordexp_t thing;
    int wordexp_ret = wordexp(command, &thing, WRDE_NOCMD);
    if (wordexp_ret != 0) {
      //printf(command);
      //printf("\n");
      //printf(thing);
      printf("opsh: Bad command or binary\n");
      errno = wordexp_ret;
      perror("wordexp");
      continue;
    }
    if (strcmp(thing.we_wordv[0], "exit") == 0) {
      printf("exit opsh\n");
      wordfree(&thing);
      exit(0);
    }
    if (strcmp(thing.we_wordv[0], "crash") == 0) {
      __asm__("ud2"); // guaranteed to crash; read the x86 instruction set
      continue;
    }
    if (strcmp(thing.we_wordv[0], "cd") == 0) {
      if (thing.we_wordv[1] != NULL) {
        if (chdir(thing.we_wordv[1]) != 0) {
          printf("opsh: failed to change directory\n");
          perror("cd");
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
            perror("opsh");
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
        signal(SIGINT, SIG_DFL);
        printf("opsh: Fail to launch command\n");
        perror("execvp");
        wordfree(&thing);
        exit(1);
      }
      wordfree(&thing); // apparently necessary
    } else if (pid > 0) {
      wait(NULL);
      wordfree(&thing);
      //free(command);
    } else {
      printf("opsh: fork failed\n");
      perror("fork");
      wordfree(&thing);
      exit(1);
    }
  }
  return 0;
}
