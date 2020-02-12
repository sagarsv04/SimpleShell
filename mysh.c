/*
 *  mysh.c
 *  Contains Shell implementation in C
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */

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


int read_shell_cmd(char *cmd_line_buff) {

  // cmd_line_size = getline(&cmd_line_buff, cmd_line_buff_size, stdin);
  fgets(cmd_line_buff, CMD_LINE_LEN, stdin);

  if (strlen(cmd_line_buff) > 0) {
    printf("You typed :: %s\n", cmd_line_buff);
    if (strcmp(cmd_line_buff, "exit\n") == 0) {
      return EXIT;
    }
  } else {
    printf("You typed Nothing\n");
  }

  return CONTINUE;
}


int split_shell_cmd_line(char *cmd_line_buff, char cmd_tokens_array[NUMBER_OF_CMD_TOKENS][CMD_LEN]) {

  if (!cmd_tokens_array) {
    fprintf(stderr, "mysh>Error :: Commands Tokens Allocation Error\n");
    return ERROR;
  }
  else {
    int token_idx = 0;
    char *token_ptr = strtok(cmd_line_buff, " ");
    while (token_ptr != NULL) {
      if (token_idx > NUMBER_OF_CMD_TOKENS) {
        fprintf(stderr, "mysh>Error :: Commands Length Exceeded\n");
      }
      else {
        if (strlen(token_ptr)>CMD_LEN) {
          fprintf(stderr, "mysh>Error :: Commands Size Exceeded :: %s\n", token_ptr);
        }
        else {
          strcpy(cmd_tokens_array[token_idx], token_ptr);
          token_idx++;
        }
      }
      printf("token is :: %s\n", token_ptr);
      token_ptr = strtok(NULL, " ");
    }
    strcpy(cmd_tokens_array[token_idx], CMD_EOF);
  }
  return CONTINUE;
}


int process_shell_cmd() {

  char *cmd_line_buff = (char*)malloc(CMD_LINE_LEN * sizeof(char));

  int func_ret;

  func_ret = read_shell_cmd(cmd_line_buff);

  if (func_ret==EXIT) {
    free(cmd_line_buff);
    exit_handler(EXIT);
  }
  else {
    // run the commands
    char cmd_tokens_array[NUMBER_OF_CMD_TOKENS][CMD_LEN];
    func_ret = split_shell_cmd_line(cmd_line_buff, cmd_tokens_array);
    if (func_ret==ERROR) {
      return ERROR;
    }
    else {
      for (int i = 0; i < NUMBER_OF_CMD_TOKENS; i++) {
        if (strcmp(cmd_tokens_array[i], CMD_EOF) == 0) {
          break;
        }
        else {
          printf("Commands Token at : %d is %s\n",i, cmd_tokens_array[i]);
        }
      }
    }
    printf("Vo VO VO\n");
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
  while (TRUE){
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
