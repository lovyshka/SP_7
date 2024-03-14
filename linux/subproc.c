#include "total.h"


int main(int argc, char * argv[]){
    int ctl, ar_2, ar_3, sum = 0;
    sscanf(argv[1], "%d", &ctl);
    sscanf(argv[2], "%d", &ar_2);
    sscanf(argv[3], "%d", &ar_3);
    printf("pid = %d\n", getpid());
    sleep(300);
    
    if (ctl == 0){
        int cnt;
        int fd_to_read = ar_2;
        int fd_to_write = ar_3;
        if (read(fd_to_read, &cnt, sizeof(int)) == -1){
            printf("Error while reading from pipe\n");
            return -1;
        }
        int tmp[cnt];
        if (read(fd_to_read, tmp, sizeof(tmp)) == -1 || close(fd_to_read) == -1){
            printf("Error while working withh pipe\n");
            return -1;
        }
        for (int i = 0; i < cnt; i++) {
            if (tmp[i] % 2 == 0) sum += tmp[i];
        }

        if (write(fd_to_write, &sum, sizeof(int)) == -1 || close(fd_to_write) == -1){
            printf("Error while working writing in pipe\n");
            return -1;
        }
    }
    else if (ctl == 1){
        key_t key = ftok("/dev/null", ar_2);
        if (key == -1){
            printf("Error while calculating key in child proc\n");
            return -1;
        }
        // printf("key from child = %d\n", key);
        int arr_len = ar_3;

        int id = shmget(key, sizeof(int) * arr_len, 0666 | IPC_CREAT);
        if (id == -1){
            printf("Error while allocatinng memory in child process\n");
            return -1;
        }
        int * arr = (int *)shmat(id, (void *) 0, 0);
        if (arr[0] == -1){
            printf("Error while attaching memory in child process\n");
            return -1;
        }
        for (int i = 0; i < arr_len; i++) if (arr[i] % 2 == 0) sum += arr[i];
        // for (int i = 0; i < arr_len; i++) printf("arr[i] = %d\n", arr[i]);
        arr[0] = sum;
        if (shmdt(arr) == -1){
            printf("Error while detaching memory in child process\n");
            return -1;
        }
    }
    return 0;
}
