all: subproc
	gcc windows_main.c -lm

subproc:
	gcc subproc.c -o subproc

clean:
	del a.exe
	del subproc.exe

rebuild: clean all