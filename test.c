#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>

//https://www.geeksforgeeks.org/ipc-shared-memory/
int main(){
    char path[] = "/tmp/example";
    int fd = open(path, O_CREAT | O_RDWR);
    close(fd);
    int arr_len = 5;
    key_t key = ftok(path, 77);
    int shmid = shmget(key, sizeof(int) * arr_len, 0666 | IPC_CREAT);

    int arr[] = (int *) shmat(shmid, (void *) 0, 0);
    for (int i = 0; i < arr_len; i++) arr[i] = i + 1;


    shmdt(arr);
    return 0;
}