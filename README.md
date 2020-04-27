# SimpleShell

A simple Shell with I/O Redirection and Processes.


Author :
============
Sagar Vishwakarma (svishwa2@binghamton.edu)

State University of New York, Binghamton


## File-Info :

1)	Makefile         - Compile the program
2)	mysh.c           - Contains implementation of a shell
3)	mysh.h           - Contains function declarations of a shell


## How to compile and run :

1)	go to terminal, cd into project directory and type 'make' to compile project

2)	Run using ./mysh

	- Now you can run your shell commands like : "ls -la" etc.

  - Type "help" to more about the commands supported by shell


## Commands Supported :

- Internal commands						: cd, pwd, echo, exit or Ctrl+C, help etc
- Clear Screen     						: clear, reset
- Input Redirection 					: grep text < input.txt
- Output Redirection 					: grep text input.txt > output.txt
- Pipe Processes 							: cat input.txt | grep text | wc -w
- Pipe + Input Redirection		: grep text < input.txt | wc -w
- Pipe + Output Redirection		: ls -la | grep text > output.txt
- Multiple Pipes							: ls -la | grep text | .. | .. | .. | .. | grep text
- Variable Substitution 			: echo $USER (substitution of max CMD_LEN characters)


## Note :

- Regex or * for string matching not supported
	eg: ls -la | grep *.h
