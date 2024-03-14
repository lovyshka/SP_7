all: subproc
	gcc test.c 

subproc: 
	gcc sub.c -o subproc

clean: 
	rm -rf a.out subproc

rebuild: clean all