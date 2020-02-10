# Make file for building application

CC = g++
# CFLAGS = -std=c++11 -lpthread
CFLAGS =
# EFLAGS = -Wall –Werror
EFLAGS =

mysh: mysh.o
	$(CC) mysh.o $(CFLAGS) $(EFLAGS) -o mysh

mysh.o: mysh.c
	$(CC) -c mysh.c $(CFLAGS) $(EFLAGS)

clean:
	rm -f *.o *.d mysh
