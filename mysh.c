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
#include <sys/types.h>
#include <sys/wait.h>


void print_shell_name(char *shell_name) {
  printf("\n%s>", shell_name);
}

void init_shell(char *shell_name) {
  printf("\033[H\033[J");
  printf("Starting :: %s ....", shell_name);
  sleep(1);
  printf("\033[H\033[J");
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


void count_pipes_and_spaces(char *cmd_line_buff, int *pipe_count, int *space_count) {

  for (int i=0; i < CMD_LINE_LEN; i++) {
    if (cmd_line_buff[i]=='|') {
      *pipe_count = *pipe_count + 1;
    }
    if (cmd_line_buff[i]==' ') {
      *space_count = *space_count + 1;
    }
  }
}


int split_shell_cmd_line(char *cmd_line_buff, char cmd_tokens_array[][CMD_LEN], int *num_of_cmd_tokens) {

  if (!cmd_tokens_array) {
    fprintf(stderr, "mysh>Error :: Commands Tokens Allocation Error\n");
    return ERROR;
  }
  else {
    char *cmd_line_buff_copy = strdup(cmd_line_buff);
    int token_idx = 0;
    char *token_ptr = strtok(cmd_line_buff, " |");
    while (token_ptr != NULL) {
      if (token_idx > *num_of_cmd_tokens) {
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
        char delim_used = cmd_line_buff_copy[token_ptr-cmd_line_buff];
        printf("CHAR >> %c\n", delim_used);
        // if (=='|') {
        //   strcpy(cmd_tokens_array[token_idx], "|");
        //   token_idx++;
        // }
      }
      // printf("CHAR >> %c\n",token_ptr);
      printf("token is :: %s\n", token_ptr);
      token_ptr = strtok(NULL, " |");
    }
    strcpy(cmd_tokens_array[token_idx], CMD_EOF);
  }

  return CONTINUE;
}


int process_shell_cmd() {

  char *cmd_line_buff = (char*)malloc(CMD_LINE_LEN * sizeof(char));
  int pipe_count = 0;
  int space_count = 0;
  int num_of_cmd_tokens = 0;
  int func_ret;

  func_ret = read_shell_cmd(cmd_line_buff);
  if (func_ret==EXIT) {
    free(cmd_line_buff);
    exit_handler(EXIT);
  }
  else {
    // run the commands
    // find if cmd_line_buff has pipe and how many
    // find if cmd_line_buff has spaces and how many
    count_pipes_and_spaces(cmd_line_buff, &pipe_count, &space_count);
    num_of_cmd_tokens = 2+space_count+2*pipe_count;

    printf("Pipe %d, space %d, array of %d\n",pipe_count, space_count, num_of_cmd_tokens);

    char cmd_tokens_array[num_of_cmd_tokens][CMD_LEN];

    func_ret = split_shell_cmd_line(cmd_line_buff, cmd_tokens_array, &num_of_cmd_tokens);
    if (func_ret==ERROR) {
      return ERROR;
    }
    else {
      for (int i = 0; i < num_of_cmd_tokens; i++) {
        printf("Commands Token at : %d is %s\n",i, cmd_tokens_array[i]);
      }
    }
  }
  free(cmd_line_buff);
  return 0;
}


int main(int argc, char const *argv[]) {

  int shell_name_len = (int)strlen(argv[0]);
  char shell_name[shell_name_len-2];
  memcpy(shell_name, argv[0]+2, shell_name_len);
  int process_ret;

  init_shell(shell_name);

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
