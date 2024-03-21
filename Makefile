all: subproc
	gcc test.c 

subproc: 
	gcc sub.c -o subproc

clean: 
	del *.exe

rebuild: clean all