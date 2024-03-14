#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>

//https://www.geeksforgeeks.org/ipc-shared-memory/

int main(){
    char path[] = "./test.c";
    // int fd = open(path, O_CREAT | O_RDWR); //создание файлика если нет
    // close(fd);
    
    int arrs[] = {2, 3};

    
    int number = 2;
    key_t key[number];
    int * arr[number];
    for (int i = 0; i < number; i++){
        
        int pid = fork();
        if (pid != 0){

            key[i] = ftok("./test.c", i);
            int shmid = shmget(key[i], sizeof(int) * arrs[i], 0666 | IPC_CREAT); 
            arr[i] = (int *) shmat(shmid, (void *) 0, 0); // "пришываем" выделенную память к нашему процессу

            for (int j = 0; j < arrs[i]; j++) arr[i][j] = i + 2; // {2, 3} {3, 4, 5}   
        }
        else {
            //subproc key arr_len
            char key_buf[100];
            char len_buf[100];
            sprintf(key_buf,"%d", i);
            sprintf(len_buf, "%d", arrs[i]);
            char * args[4] = {"./subproc", key_buf, len_buf, NULL};
            execve("./subproc", args, NULL);
        }
    }
    while(wait(NULL) != -1 || errno != ECHILD);
    int res = 0;
    for (int i = 0; i < number; i++) res += arr[i][0];
    printf("res = %d\n", res);
    shmdt(arr);
    return 0;
}