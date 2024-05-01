all: compile run

compile:
	gcc -o ./bin/main.o main.c

run:
	./bin/main.o