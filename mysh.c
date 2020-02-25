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
		// char home_path[6+(int)strlen(user)] = "/home/";
		char home_path[6+(int)strlen(user)];
		strcpy(home_path,"/home/");
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
			subs[CMD_LEN-3] = '\0';
			strcpy(cmd_token, subs);
		}
		else {
			strcpy(cmd_token, subs);
		}
	}
	if (DEBUG_PRINT) {
		printf("Value Replaced : <%s>\n", cmd_token);
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

	if (DEBUG_PRINT) {
		printf("\n");
		printf("pipe_count :%d\n", cmd_delimit->pipe_count);
		printf("space_count :%d\n", cmd_delimit->space_count);
		printf("in_re_count :%d\n", cmd_delimit->in_re_count);
		printf("out_re_count :%d\n", cmd_delimit->out_re_count);
		printf("and_count :%d\n", cmd_delimit->and_count);
		printf("total_count :%d\n", cmd_delimit->total_count);
	}
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
	if (DEBUG_PRINT) {
		printf("Delimit is : <%s>\n", delimit);
	}
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
				remove_white_spaces(token_ptr);
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


int execute_shell_single_cmd(char *cmd_line_buff, int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing Single CMD : <%s>\n", cmd_line_buff);
	}

	if (strlen(cmd_line_buff)>0) {

		char bin_path[] = "bin/";
		char user_path[] = "usr/bin/";
		// char exe_path[1+(int)strlen(bin_path)+(int)strlen(user_path)] = "/";
		char exe_path[1+(int)strlen(bin_path)+(int)strlen(user_path)];
		strcpy(exe_path,"/");

		if (strcmp(cmd_line_buff, "cd") == 0) {
			return change_dir(NULL);
		}
		else {
			if ((strcmp(cmd_line_buff, "clear")==0)||
					(strcmp(cmd_line_buff, "reset")==0)||
					(strcmp(cmd_line_buff, "touch")==0)||
					(strcmp(cmd_line_buff, "head")==0)||
					(strcmp(cmd_line_buff, "tail")==0)||
					(strcmp(cmd_line_buff, "comm")==0)||
					(strcmp(cmd_line_buff, "less")==0)||
					(strcmp(cmd_line_buff, "cmp")==0)||
					(strcmp(cmd_line_buff, "cal")==0)||
					(strcmp(cmd_line_buff, "yes")==0)||
					(strcmp(cmd_line_buff, "rev")==0)||
					(strcmp(cmd_line_buff, "wget")==0)||
					(strcmp(cmd_line_buff, "find")==0)||
					(strcmp(cmd_line_buff, "which")==0)||
					(strcmp(cmd_line_buff, "locate")==0)||
					(strcmp(cmd_line_buff, "sort")==0)||
					(strcmp(cmd_line_buff, "sudo")==0)||
					(strcmp(cmd_line_buff, "man")==0)||
					(strcmp(cmd_line_buff, "whatis")==0)||
					(strcmp(cmd_line_buff, "whoami")==0)){
				strcat(exe_path, user_path);
				strcat(exe_path, cmd_line_buff);
			}
			else {
				strcat(exe_path, bin_path);
				strcat(exe_path, cmd_line_buff);
			}

			char *args[] = {exe_path, NULL};

			if (DEBUG_PRINT) {
				printf("Executing Cmd Path : <%s>\n", args[0]);
			}

			if (pipe_FLAG==READ_FLAG) {
				// do not fork as the process is already forked
				// just handle stdout, sdtin
				execv(args[0], args);
				perror("Error :: Invalid Input.\n");
				return ERROR;
			}
			else if (pipe_FLAG==WRITE_FLAG) {
				// do not fork as the process is already forked
				// just handle stdout, sdtin
				execv(args[0], args);
				perror("Error :: Invalid Input.\n");
				return ERROR;
			}
			else {
				// create a child process using fork for space arg
				pid_t pid = fork();

				if (pid < 0) {
					perror("Error :: Failed forking child.\n");
					return ERROR;
				}
				else if (pid == 0) {
					if (FORK_SLEEP) {
						printf("Sleep of %ds in child process\n", SLEEP);
						sleep(SLEEP);
					}
					execv(args[0], args);
					perror("Error :: Invalid Input.\n");
					return ERROR;
				}
				else {
					wait(NULL);
				}
			}
		}
	}

	return CONTINUE;
}


int execute_args(char **args, int num_cmd, int pipe_FLAG) {

	if (strcmp(args[0], "cd") == 0) {
		return change_dir(args[1]);
	}
	else {

		if (DEBUG_PRINT) {
			for (int i=0; i<num_cmd; i++) {
				printf("Executing Cmd %d: <%s>\n", i, args[i]);
			}
		}

		if (pipe_FLAG==READ_FLAG) {
			// do not fork as the process is already forked
			// just handle stdout, sdtin
			execvp(args[0], args);
			perror("Error :: Invalid Input\n");
			return ERROR;
		}
		else if (pipe_FLAG==WRITE_FLAG) {
			// do not fork as the process is already forked
			// just handle stdout, sdtin
			execvp(args[0], args);
			perror("Error :: Invalid Input\n");
			return ERROR;
		}
		else {
			// create a child process using fork for space arg
			pid_t pid = fork();

			if (pid < 0) {
				perror("Error :: Failed forking child.\n");
				return ERROR;
			}
			else if (pid == 0) {
				if (FORK_SLEEP) {
					printf("Sleep of %ds in child process\n", SLEEP);
					sleep(SLEEP);
				}
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


int execute_shell_cmd_with_space(char *cmd_line_buff, DELIMIT_Count cmd_delimit, int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing Space CMD : <%s>\n", cmd_line_buff);
	}

	char space_delimit[] = " ";
	int array_size = cmd_delimit.space_count+1;
	char cmd_tokens_array[array_size][CMD_LEN];
	int token_idx = split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, space_delimit);
	if (token_idx<=cmd_delimit.space_count) {
		// means space is added after single command
		// add null at space and run again
		add_null_at_delimit(cmd_line_buff, space_delimit);
		clear_all_delimiters_count(&cmd_delimit);
		count_all_delimiters(cmd_line_buff, &cmd_delimit);
		if (cmd_delimit.space_count > 0) {
			return execute_shell_cmd_with_space(cmd_line_buff, cmd_delimit, pipe_FLAG);
		}
		else {
			return execute_shell_single_cmd(cmd_line_buff, pipe_FLAG);
		}
	}
	else {
		// replace $__ with value
		check_variable_substitution(cmd_tokens_array[1]);
		// find a better way to do this
		if (token_idx > 5) {
			fprintf(stderr, "Error :: Sorry Support upto four space delimited command.\n");
			return CONTINUE;
		}
		else {

			if (token_idx==2) {
				char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], NULL}; // this works
				return execute_args(args, token_idx, pipe_FLAG);
			}
			else if (token_idx==3) {
				char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], cmd_tokens_array[2], NULL}; // this works
				return execute_args(args, token_idx, pipe_FLAG);
			}
			else if (token_idx==4) {
				char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], cmd_tokens_array[2], cmd_tokens_array[3], NULL}; // this works
				return execute_args(args, token_idx, pipe_FLAG);
			}
			else if (token_idx==5) {
				char *args[] = {cmd_tokens_array[0], cmd_tokens_array[1], cmd_tokens_array[2], cmd_tokens_array[3], cmd_tokens_array[4], NULL}; // this works
				return execute_args(args, token_idx, pipe_FLAG);
			}
		}
	}

	return CONTINUE;
}


int execute_shell_cmd_redirection(char *cmd_line_buff, DELIMIT_Count cmd_delimit, int pipe_FLAG) {

	if ((cmd_delimit.in_re_count > 0)&&(cmd_delimit.out_re_count > 0)) {
		fprintf(stderr, "Error :: Unsupported operation with Input/Output Redirection\n");
	}
	else {
		char redirection[] = " ";
		int array_size = 1;
		char oflag[] = " ";

		if (cmd_delimit.in_re_count > 0) {
			strcpy(redirection, "<");
			strcpy(oflag, "r");
			array_size += cmd_delimit.in_re_count;
		}
		else if (cmd_delimit.out_re_count > 0) {
			strcpy(redirection, ">");
			strcpy(oflag, "w");
			array_size += cmd_delimit.out_re_count;
		}
		else {
			fprintf(stderr, "Error :: Invalid operation with Input/Output Redirection\n");
			return ERROR;
		}

		if (DEBUG_PRINT) {
			printf("Redirect Delimit is : <%s>\n", redirection);
		}

		char cmd_tokens_array[array_size][CMD_LEN];
		int token_idx = split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, redirection);
		if ((token_idx<=cmd_delimit.out_re_count)||(token_idx<=cmd_delimit.in_re_count)) {
			fprintf(stderr, "Error :: Invalid command : <%s>.\n", cmd_line_buff);
		}
		else {

			DELIMIT_Count sub_cmd_delimit;
			clear_all_delimiters_count(&sub_cmd_delimit);
			count_all_delimiters(cmd_tokens_array[0], &sub_cmd_delimit);

			if (pipe_FLAG==READ_FLAG) {
				/* code */
			}
			else if (pipe_FLAG==WRITE_FLAG) {
				/* code */
			}
			else {
				// create a child process using fork for space arg
				pid_t pid = fork();

				if (pid < 0) {
					perror("Error :: Failed forking child.\n");
					return ERROR;
				}
				else if (pid == 0) {
					if (FORK_SLEEP) {
						printf("Sleep of %ds in child process\n", SLEEP);
						sleep(SLEEP);
					}
					int file_disc;

					if (strcmp(oflag, "r") == 0) {

						file_disc = open(cmd_tokens_array[1], O_RDONLY);
						if (file_disc < 0) {
							perror("Error :: Failed reading file.\n");
							fprintf(stderr, "File : %s\n",cmd_tokens_array[1]);
							return ERROR;
						}
						else{
							dup2(file_disc, STDIN_FILENO);
							close(file_disc);
						}
						return execute_shell_cmd_with_space(cmd_tokens_array[0], sub_cmd_delimit, READ_FLAG);
					}
					else if (strcmp(oflag, "w") == 0) {

						fprintf(stderr, "File trying to create : %s\n",cmd_tokens_array[1]);
						file_disc = creat(cmd_tokens_array[1], 0644);
						if (file_disc < 0) {
							perror("Error :: Failed creating file.\n");
							fprintf(stderr, "File : %s\n",cmd_tokens_array[1]);
							return ERROR;
						}
						else{
							dup2(file_disc, STDOUT_FILENO);
							close(file_disc);
						}
						return execute_shell_cmd_with_space(cmd_tokens_array[0], sub_cmd_delimit, WRITE_FLAG);
					}
				}
				else {
					wait(NULL);
				}
			}
		}
	}

	return CONTINUE;
}


int execute_shell_cmd_pipes_loop(char *cmd_line_buff, DELIMIT_Count cmd_delimit, int pipe_FLAG) {

	char pipe_delimit[] = "|";
	int array_size = cmd_delimit.pipe_count+1;
	int pipe_fd[2*cmd_delimit.pipe_count];


	if (DEBUG_PRINT) {
		printf("Pipe Delimit is : <%s>\n", pipe_delimit);
	}

	char cmd_tokens_array[array_size][CMD_LEN];
	split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, pipe_delimit);

	/* parent creates all needed pipes at the start */
	for(int i=0; i<cmd_delimit.pipe_count; i++) {
		if(pipe(pipe_fd+i*2)<0) {
			perror("Error :: Pipe Failed.\n");
			return ERROR;
		}
	}

	for (int cmd_idx=0; cmd_idx<array_size; cmd_idx++) {

		DELIMIT_Count sub_cmd_delimit;
		clear_all_delimiters_count(&sub_cmd_delimit);
		count_all_delimiters(cmd_tokens_array[cmd_idx], &sub_cmd_delimit);

		if (DEBUG_PRINT) {
			for(int i=0; i<cmd_delimit.pipe_count; i++) {
				printf("Pipes before fork %d : R-%d, W-%d\n", cmd_idx, pipe_fd[cmd_idx*2], pipe_fd[(cmd_idx*2)+1]);
			}
		}
		// create a child process using fork for space arg
		pid_t pid = fork();

		if (pid < 0) {
			perror("Error :: Failed forking child.\n");
			return ERROR;
		}
		else if (pid == 0) {
			if (FORK_SLEEP) {
				printf("Sleep of %ds in child process\n", SLEEP);
				sleep(SLEEP);
			}
			/* child gets input from the previous command Pipe WR, if it's not the first command */
			if(cmd_idx != 0) {
				if (dup2(pipe_fd[(cmd_idx-1)*2], STDIN_FILENO) < 0) {
					perror("Error :: Dup Failed For STDIN.\n");
					printf("Cannot Dup : %d Cmd STDIN to W-%d Cmd.\n", cmd_idx, cmd_idx-1);
					return ERROR;
				}
			}
			/* child outputs to next command Pipe R, if it's not the last command */
			if(cmd_idx != array_size-1) {
				if (dup2(pipe_fd[(cmd_idx*2)+1], STDOUT_FILENO) < 0) {
					perror("Error :: Dup Failed For STDOUT.\n");
					printf("Cannot Dup : %d Cmd STDOUT to R-%d Cmd.\n", cmd_idx, cmd_idx+1);
					return ERROR;
				}
			}
			if (DEBUG_PRINT) {
				for(int i=0; i<cmd_delimit.pipe_count; i++) {
					printf("Pipes after fork %d : R-%d, W-%d\n", cmd_idx, pipe_fd[cmd_idx*2], pipe_fd[(cmd_idx*2)+1]);
				}
			}

			return execute_shell_cmd_with_space(cmd_tokens_array[cmd_idx], sub_cmd_delimit, READ_FLAG);
		}
		else {
			wait(NULL);
		}
	}

	/* parent closes all of its copies at the end */
	for(int i=0; i<(cmd_delimit.pipe_count*2); i++) {
		close(pipe_fd[i]);
	}

	return CONTINUE;
}


int execute_one_pipe(char cmd_tokens_array[][CMD_LEN], int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing One Pipe : <%s> , <%s>\n", cmd_tokens_array[0], cmd_tokens_array[1]);
	}

	int pipe_fd0[2];
	DELIMIT_Count zero_cmd_delimit;
	DELIMIT_Count one_cmd_delimit;
	clear_all_delimiters_count(&zero_cmd_delimit);
	clear_all_delimiters_count(&one_cmd_delimit);
	count_all_delimiters(cmd_tokens_array[0], &zero_cmd_delimit);
	count_all_delimiters(cmd_tokens_array[1], &one_cmd_delimit);


	if (pipe(pipe_fd0) < 0) {
		perror("Error :: Pipe Failed.\n");
		return ERROR;
	}
	pid_t pid = fork();

	if (pid < 0) {
		perror("Error :: Failed forking child.\n");
		return ERROR;
	}
	else if (pid == 0) {
		if (FORK_SLEEP) {
			printf("Sleep of %ds in child process\n", SLEEP);
			sleep(SLEEP);
		}
		// close(STDOUT_FILENO);
		// dup2(pipe_fd0[1], STDOUT_FILENO);
		if (dup2(pipe_fd0[1], STDOUT_FILENO) < 0) {
			perror("Error :: Dup Failed For STDOUT in Child 1.\n");
			return ERROR;
		}
		close(pipe_fd0[0]);
		close(pipe_fd0[1]);
		return execute_shell_cmd_with_space(cmd_tokens_array[0], zero_cmd_delimit, WRITE_FLAG);
	}
	else {
		wait(NULL); // this is important

		pid_t pid = fork();

		if (pid < 0) {
			perror("Error :: Failed forking child.\n");
			return ERROR;
		}
		else if (pid == 0) {
			if (FORK_SLEEP) {
				sleep(SLEEP);
			}
			// close(STDIN_FILENO);
			// dup2(pipe_fd0[0], STDIN_FILENO);
			if (dup2(pipe_fd0[0], STDIN_FILENO) < 0) {
				perror("Error :: Dup Failed For STDIN in Child 2.\n");
				return ERROR;
			}
			close(pipe_fd0[1]);
			close(pipe_fd0[0]);
			return execute_shell_cmd_with_space(cmd_tokens_array[1], one_cmd_delimit, READ_FLAG);
		}
		else {
			close(pipe_fd0[0]);
			close(pipe_fd0[1]);
			wait(NULL); // this is important
		}
	}

	return CONTINUE;
}


int execute_two_pipe(char cmd_tokens_array[][CMD_LEN], int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing Two Pipe : <%s> , <%s> , <%s>\n", cmd_tokens_array[0], cmd_tokens_array[1], cmd_tokens_array[2]);
	}

	int pipe_fd0[2];
	int pipe_fd1[2];
	DELIMIT_Count zero_cmd_delimit;
	DELIMIT_Count one_cmd_delimit;
	DELIMIT_Count two_cmd_delimit;
	clear_all_delimiters_count(&zero_cmd_delimit);
	clear_all_delimiters_count(&one_cmd_delimit);
	clear_all_delimiters_count(&two_cmd_delimit);
	count_all_delimiters(cmd_tokens_array[0], &zero_cmd_delimit);
	count_all_delimiters(cmd_tokens_array[1], &one_cmd_delimit);
	count_all_delimiters(cmd_tokens_array[2], &two_cmd_delimit);

	if (pipe(pipe_fd0) < 0) {
		perror("Error :: Pipe Failed.\n");
		return ERROR;
	}
	if (pipe(pipe_fd1) < 0) {
		perror("Error :: Pipe Failed.\n");
		return ERROR;
	}
	pid_t pid = fork();

	if (pid < 0) {
		perror("Error :: Failed forking child.\n");
		return ERROR;
	}
	else if (pid == 0) {
		if (FORK_SLEEP) {
			printf("Sleep of %ds in child process\n", SLEEP);
			sleep(SLEEP);
		}
		// close(STDOUT_FILENO);
		// dup2(pipe_fd0[1], STDOUT_FILENO);
		if (dup2(pipe_fd0[1], STDOUT_FILENO) < 0) {
			perror("Error :: Dup Failed For STDOUT in Child 1.\n");
			return ERROR;
		}
		close(pipe_fd0[1]);
		close(pipe_fd0[0]);
		close(pipe_fd1[1]);
		close(pipe_fd1[0]);
		return execute_shell_cmd_with_space(cmd_tokens_array[0], one_cmd_delimit, WRITE_FLAG);
	}
	else {
		wait(NULL); // this is important
		pid_t pid = fork();

		if (pid < 0) {
			perror("Error :: Failed forking child.\n");
			return ERROR;
		}
		else if (pid == 0) {
			if (FORK_SLEEP) {
				sleep(SLEEP);
			}
			// close(STDIN_FILENO);
			// dup2(pipe_fd0[0], STDIN_FILENO);
			if (dup2(pipe_fd0[0], STDIN_FILENO) < 0) {
				perror("Error :: Dup Failed For STDIN in Child 2.\n");
				return ERROR;
			}
			// close(STDOUT_FILENO);
			// dup2(pipe_fd1[1], STDOUT_FILENO);
			if (dup2(pipe_fd1[1], STDOUT_FILENO) < 0) {
				perror("Error :: Dup Failed For STDOUT in Child 2.\n");
				return ERROR;
			}
			close(pipe_fd0[1]);
			close(pipe_fd0[0]);
			close(pipe_fd1[1]);
			close(pipe_fd1[0]);
			return execute_shell_cmd_with_space(cmd_tokens_array[1], two_cmd_delimit, READ_FLAG);
		}
		else {
			wait(NULL); // this is important
			pid_t pid = fork();

			if (pid < 0) {
				perror("Error :: Failed forking child.\n");
				return ERROR;
			}
			else if (pid == 0) {
				if (FORK_SLEEP) {
					sleep(SLEEP);
				}
				// close(STDIN_FILENO);
				// dup2(pipe_fd1[0], STDIN_FILENO);
				if (dup2(pipe_fd1[0], STDIN_FILENO) < 0) {
					perror("Error :: Dup Failed For STDIN in Child 3.\n");
					return ERROR;
				}
				close(pipe_fd0[1]);
				close(pipe_fd0[0]);
				close(pipe_fd1[1]);
				close(pipe_fd1[0]);
				// check if output re direction
				return execute_shell_cmd_with_space(cmd_tokens_array[1], two_cmd_delimit, READ_FLAG);
			}
			else {
				close(pipe_fd0[1]);
				close(pipe_fd0[0]);
				close(pipe_fd1[1]);
				close(pipe_fd1[0]);
				wait(NULL); // this is important
			}
		}
	}

	return CONTINUE;
}

int execute_three_pipe(char cmd_tokens_array[][CMD_LEN], int pipe_FLAG) {


	return CONTINUE;
}

int execute_four_pipe(char cmd_tokens_array[][CMD_LEN], int pipe_FLAG) {


	return CONTINUE;
}



int execute_shell_cmd_pipes(char *cmd_line_buff, DELIMIT_Count cmd_delimit, int pipe_FLAG) {

	if (DEBUG_PRINT) {
		printf("Executing Pipe : <%s>\n", cmd_line_buff);
	}

	char pipe_delimit[] = "|";
	int array_size = cmd_delimit.pipe_count+1;

	char cmd_tokens_array[array_size][CMD_LEN];
	int token_idx = split_shell_cmd_by_delimit(cmd_line_buff, cmd_tokens_array, cmd_delimit, pipe_delimit);
	if (token_idx<=cmd_delimit.pipe_count) {
		fprintf(stderr, "Error :: Invalid command : <%s>.\n", cmd_line_buff);
	}
	else {

		if (token_idx > 5) {
			fprintf(stderr, "Error :: Sorry Support upto four pipe delimited command.\n");
			return CONTINUE;
		}
		else {

			if (token_idx==2) {
				return execute_one_pipe(cmd_tokens_array, pipe_FLAG);
			}
			else if (token_idx==3) {
				return execute_two_pipe(cmd_tokens_array, pipe_FLAG);
			}
			else if (token_idx==4) {
				return execute_three_pipe(cmd_tokens_array, pipe_FLAG);
			}
			else if (token_idx==5) {
				return execute_four_pipe(cmd_tokens_array, pipe_FLAG);
			}
		}
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
		// DELIMIT_Count *cmd_delimit = (DELIMIT_Count*)malloc(1 * sizeof(DELIMIT_Count));
		DELIMIT_Count cmd_delimit;
		clear_all_delimiters_count(&cmd_delimit);

		count_all_delimiters(cmd_line_buff, &cmd_delimit);

		if (cmd_delimit.pipe_count > 0) {
			// pipes plus spaces
			func_ret = execute_shell_cmd_pipes(cmd_line_buff, cmd_delimit, NO_FLAG);
			// func_ret = execute_shell_cmd_pipes_loop(cmd_line_buff, cmd_delimit, NO_FLAG);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
		else if ((cmd_delimit.in_re_count > 0)||(cmd_delimit.out_re_count > 0)) {
			// input-output redirection
			func_ret = execute_shell_cmd_redirection(cmd_line_buff, cmd_delimit, NO_FLAG);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
		else if (cmd_delimit.space_count > 0) {
			// only spaces
			func_ret = execute_shell_cmd_with_space(cmd_line_buff, cmd_delimit, NO_FLAG);
			if (func_ret==ERROR) {
				return ERROR;
			}
		}
		else {
			// single cmd
			func_ret = execute_shell_single_cmd(cmd_line_buff, NO_FLAG);
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
