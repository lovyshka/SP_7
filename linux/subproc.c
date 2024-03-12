#include "total.h"


int main(int argc, char * argv[]){
    int fd_to_read, fd_to_write, cnt, sum = 0;
    sscanf(argv[1], "%d", &fd_to_read);
    sscanf(argv[2], "%d", &fd_to_write);
    // printf("pid = %d\n", getpid());
    // sleep(300);
    
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
    return 0;
}
