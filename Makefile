# Make file for building application

CC = g++
CFLAGS = -Wall -Werror
DEPS = mysh.h

mysh: mysh.o
	$(CC) mysh.o $(CFLAGS) -o mysh

mysh.o: mysh.c $(DEPS)
	$(CC) -c mysh.c $(CFLAGS)

clean:
	rm -f *.o *.d mysh
