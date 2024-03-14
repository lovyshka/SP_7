#include <stdio.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>



int main(int argc, char * argv[]){

    int arr_len, kid, sum = 0;
    sscanf(argv[1], "%d", &kid);
    sscanf(argv[2], "%d", &arr_len);

    key_t key = ftok("./test.c", kid);

    int id = shmget(key, sizeof(int) * arr_len, 0666 | IPC_CREAT);
    int * arr = (int *)shmat(id, (void *) 0, 0);
    for (int i = 0; i < arr_len; i++) {
        printf("%d\n", arr[i]);
        if (arr[i] % 2 == 0) sum+=arr[i];
    }
    arr[0] = sum;
    shmdt(arr);
    return 0;
}