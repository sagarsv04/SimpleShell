#ifndef _MYSH_H_
#define _MYSH_H_



enum {
  EXIT,
  CONTINUE,
  SUCCESS,
  ERROR
};


void exit_handler(int signal);
void print_shell_name(char *shell_name);
int read_shell_cmd(char *cmd_line_buff, size_t *cmd_line_buff_size);
int process_shell_cmd();

#endif
