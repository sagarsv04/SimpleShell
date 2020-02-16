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



#define TRUE 1
#define FALSE 0


// #define NUMBER_OF_CMD_TOKENS 8
#define CMD_LINE_LEN 128
#define CMD_LEN 32
#define CMD_EOF "CMD_END"



enum {
  EXIT,
  CONTINUE,
  SUCCESS,
  FAILURE,
  ERROR
};


void exit_handler(int signal);
void print_shell_name(char *shell_name);
void init_shell(char *shell_name);
int read_shell_cmd(char *cmd_line_buff);
int process_shell_cmd();
void count_pipes_and_spaces(char *cmd_line_buff, int *pipe_count, int *space_count);
int split_shell_cmd_line(char *cmd_line_buff, char cmd_tokens_array[][CMD_LEN], int *num_of_cmd_tokens);

#endif
