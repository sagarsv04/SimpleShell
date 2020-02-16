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

void clear_shell() {
	printf("\033[H\033[J");
}


void print_help(char *shell_name) {
	printf("Help Info ....\n");
	printf("%s is a simple shell program with support for commands like.\n", shell_name);
	printf("Internal commands: cd, pwd, echo, exit\n");
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


int read_shell_cmd(char *cmd_line_buff, char *shell_name) {

	// cmd_line_size = getline(&cmd_line_buff, cmd_line_buff_size, stdin);
	fgets(cmd_line_buff, CMD_LINE_LEN, stdin);

	if (strlen(cmd_line_buff) > 0) {
		printf("You typed :: %s\n", cmd_line_buff);
		if (strcmp(cmd_line_buff, "exit\n") == 0) {
			return EXIT;
		}
		else if (strcmp(cmd_line_buff, "help\n") == 0) {
			print_help(shell_name);
		}
		else {
			// remove \n
			for (size_t i = 0; i < CMD_LINE_LEN; i++) {
				if (cmd_line_buff[i]=='\n'){
					cmd_line_buff[i] = '\0';
					break;
				}
			}
		}
	}
	else {
		printf("You typed Nothing\n");
	}

	return CONTINUE;
}


void count_all_delimiters(char *cmd_line_buff, DELIMIT_Count *cmd_delimit) {

	for (int i=0; i < CMD_LINE_LEN; i++) {

		if (cmd_line_buff[i]=='|') {
			cmd_delimit->pipe_count += 1;
		}
		else if (cmd_line_buff[i]==' ') {
			cmd_delimit->space_count += 1;
		}
		else if (cmd_line_buff[i]=='&') {
			cmd_delimit->and_count += 1;
		}
		else if (cmd_line_buff[i]=='<') {
			cmd_delimit->in_re_count += 1;
		}
		else if (cmd_line_buff[i]=='>') {
			cmd_delimit->out_re_count += 1;
		}
	}
	cmd_delimit->total_count = cmd_delimit->pipe_count+cmd_delimit->space_count+cmd_delimit->and_count+cmd_delimit->in_re_count+cmd_delimit->in_re_count+cmd_delimit->out_re_count;
}


int split_shell_cmd_line(char *cmd_line_buff, char cmd_tokens_array[][CMD_LEN], int *num_of_cmd_tokens) {

	if (!cmd_tokens_array) {
		fprintf(stderr, "mysh>Error :: Commands Tokens Allocation Error\n");
		return ERROR;
	}
	else {
		// char *cmd_line_buff_copy = strdup(cmd_line_buff);
		int token_idx = 0;
		char *token_ptr = strtok(cmd_line_buff, " ");
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
				// char delim_used = cmd_line_buff_copy[token_ptr-cmd_line_buff];
				// printf("CHAR >> %c\n", delim_used);
				// if (=='|') {
				//   strcpy(cmd_tokens_array[token_idx], "|");
				//   token_idx++;
				// }
			}
			// printf("CHAR >> %c\n",token_ptr);
			printf("token is :: %s\n", token_ptr);
			token_ptr = strtok(NULL, " ");
		}
		// cmd_tokens_array[token_idx] = CMD_EOF;
		// strcpy(cmd_tokens_array[token_idx], CMD_EOF);
	}

	return CONTINUE;
}


int split_shell_cmd_by_delimit(char *cmd_line_buff, char cmd_tokens_array[][CMD_LEN], DELIMIT_Count *cmd_delimit, char *delimit) {

	int token_idx = 0;
	char *token_ptr = strtok(cmd_line_buff, delimit);
	while (token_ptr != NULL) {
		if (token_idx > cmd_delimit->total_count) {
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
		token_ptr = strtok(NULL, delimit);
	}
	return token_idx;
}


void add_null_at_delimit(char *cmd_line_buff, char *delimit) {

	for (size_t i = 0; i < CMD_LINE_LEN; i++) {
		if (cmd_line_buff[i]==*delimit){
			cmd_line_buff[i] = '\0';
			break;
		}
	}
}


int execute_shell_single_cmd(char *cmd_line_buff) {

	char bin_path[] = "/bin/";
	strcat(bin_path, cmd_line_buff);

	char *args[] = {bin_path, NULL};

	pid_t pid = fork();

	if (pid == -1) {
		perror("Error :: Failed forking child.\n");
		return ERROR;
	}
	else if (pid == 0) {
		execv(args[0], args);
		perror("Error :: Invalid Input.\n");
		return ERROR;
	}
	else {
		wait(NULL);
	}
	return CONTINUE;
}

int execute_shell_cmd_with_space(char *cmd_line_buff, DELIMIT_Count *cmd_delimit) {

	char space_delimit = ' ';
	char cmd_tokens_array[cmd_delimit->total_count+1][CMD_LEN];
	int token_idx = split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, &space_delimit);
	if (token_idx<=cmd_delimit->total_count) {
		// means space is added after single command
		add_null_at_delimit(cmd_line_buff, &space_delimit);
		// add null at space and run as single
		return execute_shell_single_cmd(cmd_line_buff);
	}
	else {
		// create fork for space arg
		char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], NULL};

		pid_t pid = fork();

		if (pid == -1) {
			perror("Error :: Failed forking child.\n");
			return ERROR;
		}
		else if (pid == 0) {
			execvp(args[0], args);
			perror("Error :: Invalid Input.\n");
			return ERROR;
		}
		else {
			wait(NULL);
		}
	}
	return CONTINUE;
}

int execute_shell_cmd(char cmd_tokens_array[][CMD_LEN], int *num_of_cmd_tokens, int *cmd_count) {

	printf("Token size ::  %d\n", *num_of_cmd_tokens);
	// char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], NULL};


	char *args[] = {cmd_tokens_array[0], NULL};

	printf("1::<%s> ,2::<%s> ,3::<%s>\n",args[0],args[1],args[2]);

	pid_t pid = fork();

	if (pid == -1) {
		perror("Error :: Failed forking child.\n");
		return ERROR;
	}
	else if (pid == 0) {
		execvp(args[0], args);
		perror("Error :: Invalid Input.\n");
		return ERROR;
	}
	else {
		wait(NULL);
	}
	return CONTINUE;
}

int process_shell_cmd(char *shell_name) {

	char *cmd_line_buff = (char*)malloc(CMD_LINE_LEN * sizeof(char));
	// int cmd_count = 0;
	// int num_of_cmd_tokens = 0;
	int func_ret;

	func_ret = read_shell_cmd(cmd_line_buff, shell_name);
	if (func_ret==EXIT) {
		free(cmd_line_buff);
		exit_handler(EXIT);
	}
	else {

		DELIMIT_Count *cmd_delimit = (DELIMIT_Count*)malloc(1 * sizeof(DELIMIT_Count));
		printf("pipe_count :%d\n", cmd_delimit->pipe_count);
		printf("space_count :%d\n", cmd_delimit->space_count);
		printf("in_re_count :%d\n", cmd_delimit->in_re_count);
		printf("out_re_count :%d\n", cmd_delimit->out_re_count);
		printf("and_count :%d\n", cmd_delimit->and_count);
		printf("total_count :%d\n", cmd_delimit->total_count);
		count_all_delimiters(cmd_line_buff, cmd_delimit);
		printf("\n\n\n\n\n\n");
		printf("pipe_count :%d\n", cmd_delimit->pipe_count);
		printf("space_count :%d\n", cmd_delimit->space_count);
		printf("in_re_count :%d\n", cmd_delimit->in_re_count);
		printf("out_re_count :%d\n", cmd_delimit->out_re_count);
		printf("and_count :%d\n", cmd_delimit->and_count);
		printf("total_count :%d\n", cmd_delimit->total_count);

		// num_of_cmd_tokens = 1+cmd_delimit->total_count;
		// char cmd_tokens_array[num_of_cmd_tokens][CMD_LEN];

		if (cmd_delimit->pipe_count > 0) {
			// pipes plus spaces
			;
		}
		else if (cmd_delimit->space_count > 0) {
			// only spaces
			func_ret = execute_shell_cmd_with_space(cmd_line_buff, cmd_delimit);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
		else {
			// single cmd
			func_ret = execute_shell_single_cmd(cmd_line_buff);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}





		// func_ret = split_shell_cmd_line(cmd_line_buff, cmd_tokens_array, &num_of_cmd_tokens);
		// if (func_ret==ERROR) {
		// 	return ERROR;
		// }
		// else {
		// 	func_ret = execute_shell_cmd(cmd_tokens_array, &num_of_cmd_tokens, &cmd_count);
		// 	if (func_ret==ERROR) {
		// 		return ERROR;
		// 	}
		// }
		free(cmd_delimit);
	}
	free(cmd_line_buff);
	return 0;
}


int main(int argc, char const *argv[]) {

	int shell_name_len = (int)strlen(argv[0]);
	char shell_name[shell_name_len-2];
	memcpy(shell_name, argv[0]+2, shell_name_len);
	int process_ret;

	clear_shell();

	signal(SIGINT, exit_handler);
	while (TRUE){
		print_shell_name(shell_name);
		process_ret = process_shell_cmd(shell_name);
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
