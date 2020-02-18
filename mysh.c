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
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "mysh.h"


void print_shell_name(char *shell_name) {
	printf("\n%s>", shell_name);
}

void clear_shell() {
	printf("\033[H\033[J");
}


void print_help(char *shell_name) {
	printf("Help Info ....\n");
	printf("%s is a simple shell program with support for commands like.\n", shell_name);
	printf("Internal commands: cd, pwd, echo, exit or Ctrl+C, help\n");
	printf("Clear Screen     : clear, reset\n");
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


int change_dir(char *cd_path) {

	if (DEBUG_PRINT) {
		fprintf(stderr, "Change DIR <%s>.\n", cd_path);
	}

	if (cd_path==NULL) {
		// not path goeas to home user
		char *user = getenv("USER");
		char home_path[6+(int)strlen(user)] = "/home/";
		if(user!=NULL) {
			strcat(home_path, user);
		}
		if (chdir(home_path) != 0) {
			perror("Error :: chdir failed,\n");
			if (DEBUG_PRINT) {
				fprintf(stderr, "PATH <%s>.\n", home_path);
			}
			return ERROR;
		}
	}
	else {
		if (chdir(cd_path) != 0) {
			perror("Error :: chdir failed,\n");
			if (DEBUG_PRINT) {
				fprintf(stderr, "PATH <%s>.\n", cd_path);
			}
			return ERROR;
		}
	}
	return CONTINUE;
}


void check_variable_substitution(char *cmd_token) {

	if (cmd_token[0]=='$') {
		char *subs = getenv(cmd_token+1);

		if (subs==NULL) {
			if (DEBUG_PRINT) {
				fprintf(stderr, "No Substitution for : <%s>.\n", cmd_token);
			}
			strcpy(cmd_token, "\0");
		}
		else if (strlen(subs)>CMD_LEN) {
			subs[strlen(subs)-CMD_LEN] = '\0';
			strcpy(cmd_token, subs);
		}
		else {
			strcpy(cmd_token, subs);
		}
	}
}


int read_shell_cmd(char *cmd_line_buff, char *shell_name) {

	fgets(cmd_line_buff, CMD_LINE_LEN, stdin);

	if (strlen(cmd_line_buff) > 0) {
		if (DEBUG_PRINT) {
			printf("You typed :: <%s>\n", cmd_line_buff);
		}
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
		if (DEBUG_PRINT) {
			printf("You typed Nothing\n");
		}
	}

	return CONTINUE;
}


void clear_all_delimiters_count(DELIMIT_Count *cmd_delimit) {

	// Initialize struct with zero
	cmd_delimit->pipe_count = 0;
	cmd_delimit->space_count = 0;
	cmd_delimit->in_re_count = 0;
	cmd_delimit->out_re_count = 0;
	cmd_delimit->and_count = 0;
	cmd_delimit->total_count = 0;
}


void count_all_delimiters(char *cmd_line_buff, DELIMIT_Count *cmd_delimit) {

	int cmd_len = strlen(cmd_line_buff);

	for (int i=0; i < cmd_len; i++) {

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


void add_null_at_delimit(char *cmd_line_buff, char *delimit) {

	for (size_t i = 0; i < CMD_LINE_LEN; i++) {
		if (cmd_line_buff[i]==*delimit){
			cmd_line_buff[i] = '\0';
			break;
		}
	}
}


void remove_white_spaces(char* cmd_token_buff) {

	if (cmd_token_buff[strlen(cmd_token_buff)-1]==' ') {
		cmd_token_buff[strlen(cmd_token_buff)-1]='\0';
	}
	if(cmd_token_buff[0]==' ') {
		memmove(cmd_token_buff, cmd_token_buff+1, strlen(cmd_token_buff));
	}
}


int split_shell_cmd_by_delimit(char *cmd_line_buff, char cmd_tokens_array[][CMD_LEN], DELIMIT_Count cmd_delimit, char *delimit) {

	int token_idx = 0;
	char *token_ptr = strtok(cmd_line_buff, delimit);
	while (token_ptr != NULL) {
		if (token_idx > cmd_delimit.total_count) {
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
		if (DEBUG_PRINT) {
			printf("Token is : <%s>\n", token_ptr);
		}
		token_ptr = strtok(NULL, delimit);
	}
	return token_idx;
}


int execute_shell_single_cmd(char *cmd_line_buff, int *pipe_one, int *pipe_two, int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing Single CMD : <%s>\n", cmd_line_buff);
	}

	if (strlen(cmd_line_buff)>0) {

		char bin_path[] = "bin/";
		char user_path[] = "usr/bin/";
		char exe_path[1+(int)strlen(bin_path)+(int)strlen(user_path)] = "/";

		if (strcmp(cmd_line_buff, "cd") == 0) {
			return change_dir(NULL);
		}
		else {
			if ((strcmp(cmd_line_buff, "clear")==0)||
					(strcmp(cmd_line_buff, "reset")==0)||
					(strcmp(cmd_line_buff, "whoami")==0)) {
				strcat(exe_path, user_path);
				strcat(exe_path, cmd_line_buff);
			}
			else {
				strcat(exe_path, bin_path);
				strcat(exe_path, cmd_line_buff);
			}

			if (DEBUG_PRINT) {
				printf("Executing Path : <%s>\n", exe_path);
			}

			char *args[] = {exe_path, NULL};
			pid_t pid = fork();

			if (pid < 0) {
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
		}
	}

	return CONTINUE;
}


int execute_shell_cmd_with_space(char *cmd_line_buff, DELIMIT_Count cmd_delimit, int *pipe_one, int *pipe_two, int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing Space CMD : <%s>\n", cmd_line_buff);
	}

	char space_delimit = ' ';
	char cmd_tokens_array[cmd_delimit.total_count+1][CMD_LEN];
	int token_idx = split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, &space_delimit);
	if (token_idx<=cmd_delimit.total_count) {
		// means space is added after single command
		add_null_at_delimit(cmd_line_buff, &space_delimit);
		// add null at space and run as single
		return execute_shell_single_cmd(cmd_line_buff, pipe_one, pipe_two, pipe_FLAG);
	}
	else {
		// replace $__ with value
		check_variable_substitution(cmd_tokens_array[1]);

		char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], NULL};

		if (strcmp(args[0], "cd") == 0) {
			return change_dir(args[1]);

		}
		else {
			// create fork for space arg
			pid_t pid = fork();

			if (pid < 0) {
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
	}

	return CONTINUE;
}


int execute_shell_cmd_redirection(char *cmd_line_buff, DELIMIT_Count cmd_delimit, int *pipe_one, int *pipe_two, int pipe_FLAG) {

	if ((cmd_delimit.in_re_count > 0)&&(cmd_delimit.out_re_count > 0)) {
		fprintf(stderr, "Error :: Invalid operation with Input/Output Redirection\n");
	}
	else {
		char redirection = ' ';
		int array_size = 1;
		int oflag;
		int file_descriptor = 0;
		int new_file_descriptor;

		if (cmd_delimit.in_re_count > 0) {
			redirection = '<';
			oflag = O_RDONLY;
			new_file_descriptor = 0;
			array_size += cmd_delimit.in_re_count;
		}
		else if (cmd_delimit.out_re_count > 0) {
			redirection = '>';
			oflag = O_WRONLY;
			new_file_descriptor = 1;
			array_size += cmd_delimit.out_re_count;
		}
		else {
			fprintf(stderr, "Error :: Invalid operation with Input/Output Redirection\n");
			return ERROR;
		}

		char cmd_tokens_array[array_size][CMD_LEN];
		int token_idx = split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, &redirection);
		remove_white_spaces(cmd_tokens_array[0]);
		remove_white_spaces(cmd_tokens_array[1]);

		DELIMIT_Count sub_cmd_delimit;
		clear_all_delimiters_count(&sub_cmd_delimit);

		if (DEBUG_PRINT) {
			printf("sub_pipe_count :%d\n", sub_cmd_delimit.pipe_count);
			printf("sub_space_count :%d\n", sub_cmd_delimit.space_count);
			printf("sub_in_re_count :%d\n", sub_cmd_delimit.in_re_count);
			printf("sub_out_re_count :%d\n", sub_cmd_delimit.out_re_count);
			printf("sub_and_count :%d\n", sub_cmd_delimit.and_count);
			printf("sub_total_count :%d\n", sub_cmd_delimit.total_count);
		}

		count_all_delimiters(cmd_tokens_array[0], &sub_cmd_delimit);

		if (DEBUG_PRINT) {
			printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			printf("sub_pipe_count :%d\n", sub_cmd_delimit.pipe_count);
			printf("sub_space_count :%d\n", sub_cmd_delimit.space_count);
			printf("sub_in_re_count :%d\n", sub_cmd_delimit.in_re_count);
			printf("sub_out_re_count :%d\n", sub_cmd_delimit.out_re_count);
			printf("sub_and_count :%d\n", sub_cmd_delimit.and_count);
			printf("sub_total_count :%d\n", sub_cmd_delimit.total_count);
		}


		// create a child process
		// create pipe_two
		// if pipe_FLAG is read, read from pipe_one[1]
		// if pipe flag is write, write to pipe_one[0]
		// else create pipe_one and pass to func with repective pipe_FLAG

		// file_descriptor = open(cmd_tokens_array[1], oflag);
		//
		// dup2(file_descriptor,new_file_descriptor);
		//
		// close(file_descriptor);

		execute_shell_cmd_with_space(cmd_tokens_array[0], sub_cmd_delimit, pipe_one, pipe_two, pipe_FLAG);

	// 	for (int i = 0; i < array_size; i++) {
	// 		printf("Redirection Token : %s\n",cmd_tokens_array[i]);
	// 	}
		printf("Redirection Token Idx : %d\n", token_idx);
		printf("oflag : %d\n", oflag);
		printf("file_descriptor : %d\n", file_descriptor);
		printf("new_file_descriptor : %d\n", new_file_descriptor);
	}

	return CONTINUE;
}


int process_shell_cmd(char *shell_name) {

	char *cmd_line_buff = (char*)malloc(CMD_LINE_LEN * sizeof(char));
	int func_ret;

	func_ret = read_shell_cmd(cmd_line_buff, shell_name);
	if (func_ret==EXIT) {
		free(cmd_line_buff);
		exit_handler(EXIT);
	}
	else {
		// dummy pipes
		int pipe_one[2];
		int pipe_two[2];

		// DELIMIT_Count *cmd_delimit = (DELIMIT_Count*)malloc(1 * sizeof(DELIMIT_Count));
		DELIMIT_Count cmd_delimit;
		clear_all_delimiters_count(&cmd_delimit);

		if (DEBUG_PRINT) {
			printf("pipe_count :%d\n", cmd_delimit.pipe_count);
			printf("space_count :%d\n", cmd_delimit.space_count);
			printf("in_re_count :%d\n", cmd_delimit.in_re_count);
			printf("out_re_count :%d\n", cmd_delimit.out_re_count);
			printf("and_count :%d\n", cmd_delimit.and_count);
			printf("total_count :%d\n", cmd_delimit.total_count);
		}

		count_all_delimiters(cmd_line_buff, &cmd_delimit);

		if (DEBUG_PRINT) {
			printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
			printf("pipe_count :%d\n", cmd_delimit.pipe_count);
			printf("space_count :%d\n", cmd_delimit.space_count);
			printf("in_re_count :%d\n", cmd_delimit.in_re_count);
			printf("out_re_count :%d\n", cmd_delimit.out_re_count);
			printf("and_count :%d\n", cmd_delimit.and_count);
			printf("total_count :%d\n", cmd_delimit.total_count);
		}

		// num_of_cmd_tokens = 1+cmd_delimit.total_count;
		// char cmd_tokens_array[num_of_cmd_tokens][CMD_LEN];

		if (cmd_delimit.pipe_count > 0) {
			// pipes plus spaces
			;
		}
		else if ((cmd_delimit.in_re_count > 0)||(cmd_delimit.out_re_count > 0)) {
			// input-output redirection
			func_ret = execute_shell_cmd_redirection(cmd_line_buff, cmd_delimit, pipe_one, pipe_two, NO_FLAG);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
		else if (cmd_delimit.space_count > 0) {
			// only spaces
			func_ret = execute_shell_cmd_with_space(cmd_line_buff, cmd_delimit, pipe_one, pipe_two, NO_FLAG);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
		else {
			// single cmd
			func_ret = execute_shell_single_cmd(cmd_line_buff, pipe_one, pipe_two, NO_FLAG);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
	}

	free(cmd_line_buff);

	if (DEBUG_PRINT) {
		printf("\n>>>>> Done process_shell_cmd BYE BYE BYE >>>>>\n");
	}

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
