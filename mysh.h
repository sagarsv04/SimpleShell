/*
 *  mysh.c
 *  Contains Shell implementation in C
 *
 *  Author :
 *  Sagar Vishwakarma (svishwa2@binghamton.edu)
 *  State University of New York, Binghamton
 */

#ifndef _MYSH_H_
#define _MYSH_H_



#define NUMBER_OF_CMD_TOKENS 32
#define CMD_LINE_LEN 128


enum {
  EXIT,
  CONTINUE,
  SUCCESS,
  FAILURE,
  ERROR
};


void exit_handler(int signal);
void print_shell_name(char *shell_name);
int read_shell_cmd(char *cmd_line_buff, size_t *cmd_line_buff_size);
int process_shell_cmd();
int split_shell_cmd_line(char *cmd_line_buff, char **cmd_tokens_array);

#endif
