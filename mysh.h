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

#define DEBUG_PRINT TRUE


// #define NUMBER_OF_CMD_TOKENS 8
#define CMD_LINE_LEN 128
#define CMD_LEN 32
#define CMD_EOF NULL



enum {
  EXIT,
  CONTINUE,
  SUCCESS,
  FAILURE,
  ERROR
};


typedef struct DELIMIT_Count {
  int pipe_count;
  int space_count;
  int in_re_count;
  int out_re_count;
  int and_count;
  int total_count;
} DELIMIT_Count;


void print_shell_name(char *shell_name);
void clear_shell();
void print_help(char *shell_name);
void exit_handler(int signal);

int change_dir(char *cd_path);
int read_shell_cmd(char *cmd_line_buff, char *shell_name);
void count_all_delimiters(char *cmd_line_buff, DELIMIT_Count *cmd_delimit);
void add_null_at_delimit(char *cmd_line_buff, char *delimit);

int split_shell_cmd_by_delimit(char *cmd_line_buff, char cmd_tokens_array[][CMD_LEN], DELIMIT_Count *cmd_delimit, char *delimit);
int execute_shell_single_cmd(char *cmd_line_buff);
int execute_shell_cmd_with_space(char *cmd_line_buff, DELIMIT_Count *cmd_delimit);

int process_shell_cmd(char *shell_name);

#endif
