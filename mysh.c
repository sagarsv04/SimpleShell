#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "mysh.h"
#include <sys/types.h>
#include <sys/wait.h>


static volatile int terminate = 0;


void exit_handler(int signal) {
  printf("You have presses Ctrl-C\n");
  terminate = 1;
}


void print_shell_name(char *shell_name) {
  printf("\n%s>", shell_name);
  read_shell_cmd();
}


int read_shell_cmd() {

  char *cmd_line_buff = NULL;
  size_t cmd_line_buff_size = 0;
  ssize_t cmd_line_size;

  cmd_line_size = getline(&cmd_line_buff, &cmd_line_buff_size, stdin);

  if (cmd_line_size >= 0) {
    printf("You typed :: %s\n", cmd_line_buff);
  } else {
    printf("You typed Nothing\n");
  }
  free(cmd_line_buff);

  return 0;
}


int main(int argc, char const *argv[]) {

  int shell_name_len = (int)strlen(argv[0]);
  char shell_name[shell_name_len-2];
  memcpy(shell_name, argv[0]+2, shell_name_len);

  signal(SIGINT, exit_handler);
  while (!terminate){
    print_shell_name(shell_name);
    sleep(1);
  }

  return 0;
}
