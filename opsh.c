#include <unistd.h>
#include <wordexp.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main() {
  printf("opsh: orca's Primitive SHell (Op-Shell) 1.1\n");
  char * command;
  for (;;) {
    char cmd[255];
    printf("[opsh-1]$ "); // prompt. customize as you want
    command = fgets(cmd, sizeof(cmd), stdin);
    if (!command) {
        exit(0);
    }
    command[strcspn(command, "\n")] = '\0'; // null-terminate newlines when you enter the command
    if (*command == '\0') {
        continue;
    }
    wordexp_t thing;
    if (wordexp(cmd, &thing, 0) != 0) {
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
