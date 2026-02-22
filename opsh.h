#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <pwd.h>
#include <stdlib.h>

char prompt[2048];

void ignore_sigint() { write(STDOUT_FILENO, "\n", 1); write(STDOUT_FILENO, prompt, strlen(prompt)); }

void sighup_notify() {
  printf("\nopsh: received signal SIGHUP, terminating\n");
  exit(1);
}

void sigquit_notify() {
  printf("\nopsh: SysRq or system shutdown\n");
  exit(1);
}

void sigill_notify() {
  const char *crash_msg = "\nopsh: CRITICAL HARDWARE ERROR\n";
  write(STDERR_FILENO, crash_msg, strlen(crash_msg));
  abort();
}

void sigsegv_notify() {
  const char *crash_msg = "\nopsh: process RAM read/write failure: likely\n\t- NULL pointer\n\t- modified/corrupt instructions or data\n\t- uncaught code failure\n";
  write(STDERR_FILENO, crash_msg, strlen(crash_msg));
  abort();
}

char *write_prompt() {
  char hostname[128];
  char wd[1024];
  getcwd(wd, 1023);
  struct passwd *p = getpwuid(getuid());
  /*
  struct passwd {
          char *pw_name; // login name
          char *pw_passwd; // encrypted password, unsupported
          uid_t pw_uid; // user ID
          gid_t pw_gid; // group ID
          time_t pw_change; // password change time, unsupported
          char *pw_class; // access class(?), unsupported
          char *pw_gecos; // full user name
          char *pw_dir; // home directory - what shells encode as `~`
          char *pw_shell; // home shell - usually /bin/bash or /bin/zsh
          time_t pw_expire; // when the password expires
  };
  */
  if (!p) {
    printf("opsh: failure to get current user");
    exit(1);
  }
  gethostname(hostname, 127);
  snprintf(prompt, 2047, "[\x1b[38;5;190m%s\x1b[0m@\x1b[38;5;190m%s\x1b[38;5;210m:\x1b[38;5;87m%s\x1b[0m]$ ", p->pw_name, hostname, wd);
  return prompt;
}
