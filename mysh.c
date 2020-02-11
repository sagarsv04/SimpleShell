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


int split_shell_cmd_line(char *cmd_line_buff, char **cmd_tokens_array) {

  char *token;
  int token_idx = 0;
  // int tokens_array_size = NUMBER_OF_CMD_TOKENS;

  if (!cmd_tokens_array) {
    fprintf(stderr, "mysh>Error :: CMD Tokens Allocation Error\n");
    return ERROR;
  }
  else {
    token = strtok(cmd_line_buff, " ");
    while (token != NULL) {
      cmd_tokens_array[token_idx] = token;
      token_idx++;
      // if (token_idx >= NUMBER_OF_CMD_TOKENS) {
      //   tokens_array_size += NUMBER_OF_CMD_TOKENS;
      //   cmd_tokens_array = (char*)realloc(cmd_tokens_array, tokens_array_size * sizeof(char*));
      //   if (!cmd_tokens_array) {
      //     fprintf(stderr, "mysh>Error :: CMD Tokens Allocation Error\n");
      //     return ERROR;
      //   }
      // }
      token = strtok(NULL, " ");
      printf("token is %s\n", token);
    }
    cmd_tokens_array[token_idx] = NULL;
  }
  return CONTINUE;
}


int process_shell_cmd() {

  char *cmd_line_buff = (char*)malloc(CMD_LINE_LEN * sizeof(char));
  size_t cmd_line_buff_size = 0;
  int func_ret;

  func_ret = read_shell_cmd(cmd_line_buff, &cmd_line_buff_size);

  if (func_ret==EXIT) {
    free(cmd_line_buff);
    exit_handler(EXIT);
  }
  else {
    // run the commands
    char **cmd_tokens_array = (char**)malloc(NUMBER_OF_CMD_TOKENS * sizeof(char*));
    func_ret = split_shell_cmd_line(cmd_line_buff, cmd_tokens_array);
    if (func_ret==ERROR) {
      free(cmd_tokens_array);
      return ERROR;
    }
    else {
      // print tokens
      int size = sizeof(cmd_tokens_array)/sizeof(cmd_tokens_array[0]);
      printf("CMD size %lu\n", sizeof(cmd_tokens_array));
      printf("CMD[] size %lu\n", sizeof(cmd_tokens_array[0]));
      for (int i = 0; i < size; i++) {
        printf("CMD Token at : %d is %s\n",i, cmd_tokens_array[i]);
      }

    }
    printf("Vo VO VO\n");
    free(cmd_tokens_array);
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
