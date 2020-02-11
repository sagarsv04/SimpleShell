#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "mysh.h"
// #include <sys/types.h>
// #include <sys/wait.h>


void print_shell_name(char *shell_name) {
  printf("\n%s>", shell_name);
}


void exit_handler(int signal) {
  if (signal==EXIT) {
    printf("Exiting ...!\n");
  }
  else {
    printf("You have presses Ctrl-C\n");
  }
  exit(EXIT);
}


int read_shell_cmd(char *cmd_line_buff, size_t *cmd_line_buff_size) {

  ssize_t cmd_line_size;
  cmd_line_size = getline(&cmd_line_buff, cmd_line_buff_size, stdin);

  if (cmd_line_size >= 0) {
    printf("You typed :: %s\n", cmd_line_buff);
    if (strcmp(cmd_line_buff, "exit\n") == 0) {
      return EXIT;
    }
  } else {
    printf("You typed Nothing\n");
  }

  return CONTINUE;
}


int process_shell_cmd() {

  char *cmd_line_buff = NULL;
  size_t cmd_line_buff_size = 0;
  int read_ret;

  read_ret = read_shell_cmd(cmd_line_buff, &cmd_line_buff_size);

  if (read_ret==EXIT) {
    free(cmd_line_buff);
    exit_handler(EXIT);
  }
  else {
    ;// run the commands
  }

  free(cmd_line_buff);
  return 0;
}


int main(int argc, char const *argv[]) {

  int shell_name_len = (int)strlen(argv[0]);
  char shell_name[shell_name_len-2];
  memcpy(shell_name, argv[0]+2, shell_name_len);
  int process_ret;

  signal(SIGINT, exit_handler);
  while (true){
    print_shell_name(shell_name);
    process_ret = process_shell_cmd();
    if (process_ret == ERROR) {
      fprintf(stderr, "Error :: Sorry I Failed You :(\n");
      exit(ERROR);
    }
    else {
      continue;
    }
  }

  return 0;
}
