all: subproc
	gcc linux_main.c -g -lm 

subproc: 
	gcc subproc.c -g -o subproc

clean:
	rm a.out subproc

rebuild: clean all