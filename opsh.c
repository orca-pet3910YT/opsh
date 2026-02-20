#include <unistd.h>
#include <wordexp.h>
#include <stdio.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>

char prompt[512];

void ignore_sigint(int sig) { write(STDOUT_FILENO, "\n", 1); write(STDOUT_FILENO, prompt, strlen(prompt)); }

void sighup_notify() {
  printf("\nopsh: received signal SIGHUP, terminating\n");
  exit(1);
}

void sigquit_notify() {
  printf("\nSysRq or system shutdown\n");
  exit(1);
}

char *write_prompt() {
  char hostname[128];
  char wd[1024];
  char *path = getcwd(wd, 1023);
  struct passwd *p = getpwuid(getuid());
  if (!p) {
    printf("opsh: failure to get current user");
    exit(1);
  }
  gethostname(hostname, 127);
  snprintf(prompt, 511, "[\x1b[38;5;190m%s\x1b[0m@\x1b[38;5;190m%s\x1b[38;5;210m:\x1b[38;5;87m%s\x1b[0m]$ ", p->pw_name, hostname, wd);
  return prompt;
}

int main(int argc, char **argv) {
  if (argv[1]) {
    if (!strcmp(argv[1], "--cat")) {
      printf("meow\n"); // we need humor
      exit(0);
    }
  }
  signal(SIGINT, ignore_sigint);
  signal(SIGHUP, sighup_notify);
  signal(SIGQUIT, sigquit_notify);
  printf("opsh: orca's Primitive SHell (Op-Shell) 1.5\n");
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
    if (wordexp(command, &thing, 0) != 0) {
      //printf(command);
      //printf("\n");
      //printf(thing);
      printf("opsh: Bad command or binary\n");
      perror("wordexp");
      continue;
    }
    if (strcmp(thing.we_wordv[0], "exit") == 0) {
      printf("exit opsh\n");
      wordfree(&thing);
      exit(0);
    }
    if (strcmp(thing.we_wordv[0], "crash") == 0) {
      __asm__("ud2");
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
