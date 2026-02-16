#include <unistd.h>
#include <wordexp.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

int main() {
  printf("opsh: orca's Primitive SHell (Op-Shell)\n");
  char * command;
  for (;;) {
    char cmd[255];
    printf("[opsh-1]$ "); // prompt. customize as you want
    command = fgets(cmd, sizeof(cmd), stdin);
    command[strcspn(command, "\n")] = '\0'; // null-terminate newlines when you enter the command
    wordexp_t thing;
    if (wordexp(cmd, &thing, 0) != 0) {
        printf("opsh: Bad command or binary\n");
        wordfree(&thing);
        continue;
    }
    if (strcmp(command, "exit") == 0) {
        printf("exit opsh\n");
        wordfree(&thing);
        exit(0);
    }
    pid_t pid = fork();
    if (pid == 0) {
      if (execvp(thing.we_wordv[0],
          thing.we_wordv) == -1) {
          printf("opsh: Fail to launch command\n");
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
