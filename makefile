all: clean compile

compile: main.c fileManager.c
	@gcc -o fileManager main.c fileManager.c -Wall -Wextra -pedantic

run: compile
	@./fileManager

clean:
	@rm -f *.o fileManager log.txt