#include "total.h"


//fd[2]:
//fd[0] - read
//fd[1] - write

#define PROC_NUM 2

int main(int argc, char *argv[]){
    if (check_argc(argc) == 1) return -1; //checek argc

    int number_of_processes = from_string_to_int(argv[2]); 
    int ipc_ctl = from_string_to_int(argv[3]);
    // printf("ipc_ctl = %d", ipc_ctl);
    FILE * fp = fopen(argv[1], "r");
    if (lovit_cal(fp, number_of_processes, ipc_ctl) == 1) return -1; //check valid fp and number_of_processes 
    
    int data_cnt = get_number_of_input(fp);
    if (check_data(data_cnt) == 1) return -1; //validate number of data in file
    
    body(fp, number_of_processes, data_cnt, ipc_ctl);

    fclose(fp);     
    return 0;
}

void body(FILE * fp, int number_of_processes, int data_cnt, int ipc_ctl){
    check_and_reduce(&number_of_processes, data_cnt);

    int tmp, arr_len = 0, sum = 0;
    int * arr = (int *) calloc(1, sizeof(int)); 

    while (fscanf(fp, "%d", &tmp) == 1){

        arr = (int *)realloc(arr, sizeof(int) *(arr_len + 1));
        arr[arr_len] = tmp;
        arr_len++;
    }
    controller(ipc_ctl, number_of_processes, arr_len, arr, &sum);
    printf("sum = %d\n", sum);
}

void controller(int ipc_ctl, int number_of_processes, int arr_len, int * arr, int * sum){
    if (ipc_ctl == 0){
        printf("Work with pipes was choosen\n");
        work_with_pipes(number_of_processes, arr_len, arr, sum);
    }else if (ipc_ctl == 1) {
        printf("Work with shared memory was choosen\n");
        work_with_shared_memory(number_of_processes, arr_len, arr, sum);
    }
}


int work_with_shared_memory(int number_of_process, int arr_len, int * arr, int * totoal_sum){
    int index = 0, sum = 0;
    int * distribution = divided_properly(number_of_process, arr_len);
    int keys[number_of_process];
    int shmids[number_of_process];
    int * tmp_buf[number_of_process];

    for (int i = 0; i < number_of_process; i++){    
        keys[i] = ftok("/dev/null", 77 + i);  // создали идентификатор   
        if (keys[i] == -1){
            printf("Error while calculating keys\n");
            return -1;
        }
        shmids[i] = shmget(keys[i], sizeof(int) * distribution[i], 0666 | IPC_CREAT); //выделяется память
        if (shmids[i] == -1) {
            perror("shmget");
            exit(EXIT_FAILURE);
        }

        tmp_buf[i] = (int *) shmat(shmids[i], (void *) 0, 0);
        if (tmp_buf[i] == (int *) -1) {
            perror("shmat");
            exit(EXIT_FAILURE);
        }

        for (int j = 0; j < distribution[i]; j++){
            tmp_buf[i][j] = arr[index];
            index++;
        }
    }
    for (int i = 0; i < number_of_process; i++){
        int pid = fork();
        if (pid == 0){
            char ctl[] = "1";
            char key_buf[100];
            char len_buf[100];
            sprintf(key_buf, "%d", 77 + i);
            sprintf(len_buf, "%d", distribution[i]);
            char * args[] = {"./subproc", ctl, key_buf, len_buf, NULL};
            if (execve("./subproc", args, NULL) == -1) {
                printf("Error while exec\n");
                return -1;
            }
        }
        else if (pid == -1){
            printf("Error while forking\n");
            return -1;
        }
    }

    while(wait(NULL) != -1 || errno != ECHILD);
    for (int h = 0; h < number_of_process; h++){
        sum += tmp_buf[h][0];
        if (shmdt(tmp_buf[h]) == -1){
            printf("Error while detaching memory\n");
            return -1;
        }
    }

    free(distribution);
    free(arr);
    
    *totoal_sum = sum;

    //сбор артефактов
}

int work_with_pipes(int number_of_process, int arr_len, int * arr, int * total_sum){
    int fd_from_p_to_ch[number_of_process][2];
    int fd_from_ch_to_p[number_of_process][2];
    int * distribution = divided_properly(number_of_process, arr_len);
    int index = 0, sum = 0;

    for (int i = 0; i < number_of_process; i++){
        if (pipe(fd_from_ch_to_p[i]) == -1 || pipe(fd_from_p_to_ch[i]) == -1){  // создали трубы
            printf("Error while creating pipe\n");
            return -1;
        }
        int pid = fork();//форкнулись
        if (pid == -1){
            printf("Error while forking\n");
            return -1;
        }
        else if (pid != 0){ // в родительском распихиваем данные по трубам
            if (close(fd_from_p_to_ch[i][0]) == -1 || close(fd_from_ch_to_p[i][1]) == -1) {
                printf("Error while closing fd");
                return -1;
            }
            
            int tmp_buf[distribution[i]];

            for (int j = 0; j < distribution[i]; j++){
                tmp_buf[j] = arr[index];
                index++;
            }

            if (write(fd_from_p_to_ch[i][1], &distribution[i], sizeof(int)) == -1 || write(fd_from_p_to_ch[i][1], tmp_buf, sizeof(int) * distribution[i]) == -1){
                printf("Error while writing\n");
                return -1;
            }
            if (close(fd_from_p_to_ch[i][1]) == -1){
                printf("Error while closing fd\n");
                return -1;
            }
        }
        else { // в дочке экзекаемся
            if (close(fd_from_p_to_ch[i][1]) == -1 || close(fd_from_ch_to_p[i][0]) == -1) {
                printf("Error while closing fd\n");
                return -1;
            }
            char ctl[] = "0";
            char fd_to_read[100];
            char fd_to_write[100];
            sprintf(fd_to_read, "%d", fd_from_p_to_ch[i][0]);
            sprintf(fd_to_write, "%d", fd_from_ch_to_p[i][1]);
            char *args[] = {"./subproc", ctl, fd_to_read, fd_to_write, NULL};
            if (execve("./subproc", args, NULL) == -1) {
                printf("Error while exec\n");
                return -1;
            }
        }
           
    }
    while(wait(NULL) != -1 || errno != ECHILD);
    
    free(distribution);
    free(arr);

    sum =  get_total_sum(number_of_process, fd_from_ch_to_p);
    *total_sum = sum;
}

int get_total_sum(int number_of_process, int (* fd_from_ch_to_p)[2]){
    int sum = 0;
    for (int j = 0; j < number_of_process; j++){
        int tmp;
        if (read(fd_from_ch_to_p[j][0], &tmp, sizeof(int)) == -1){
            printf("Error while reading from pipe\n");
            return -1;
        }
        sum += tmp;
        if (close(fd_from_ch_to_p[j][0]) == -1){
            printf("Error while closing fd\n");
            return -1;
        }
    }
    return sum;
}

int get_number_of_input(FILE * fp){
    int cnt = 0;
    int tmp;
    while (fscanf(fp, "%d", &tmp) == 1){
        cnt++;
    }
    rewind(fp);
    return cnt;
}


int from_string_to_int(char * arg){
    int number = 0;
    size_t len = strlen(arg);
    for (int i = 0; i < len; ++i) {
        if (!(isdigit(arg[i]))){
            number = -1;
        }
    }
    if (number != -1) {
        sscanf(arg, "%d", &number);
    }
    return number;
}



int check_argc(int argc){
    int flag = 0;
    if (argc != 4){
        flag = 1;
        printf("Invalid number of arguments: see usage\n ./a.out [input file] [nuber of child process] [0 - pipes, 1 - shared mamory]\n");
    }
    return flag;
}

int check_data(int data_cnt){
    int flag = 0;
    if (data_cnt <= 1){
        flag = 1;
        printf("Empty file given or there are less then 2 numbers\n");
    }
    return flag;
}


int lovit_cal(FILE * fp, int number_of_processes, int ipc_ctl){
    int flag = 0;
    if (fp == NULL){
        flag = 1;
        printf("No such file\n");
    }
    else if (number_of_processes <= 0 || number_of_processes >= 32750){
        flag = 1;
        printf("Invalid parameter file or number or children process given, be careful, that maximum number of threads is 32750\nAlso check the syntax, it has to be ./a.out [input file] [number of threads]\n");
    }
    else if (ipc_ctl != 0 && ipc_ctl != 1){
        flag = 1;
        printf("Invaild ipc_ctl flag, it must be 0 - for using pipes or 1 - for using shared memory\n");
    }
    return flag;
}

void check_and_reduce(int * number_of_processes, int arr_len){
    if (*number_of_processes > ((float) arr_len) / 2.0){
        *number_of_processes = (int)floor(arr_len / 2);
        printf("number of processes were reduced to %d\n", *number_of_processes);
    }
}

/*
    логика должна быть следующей: если количество чисел нацело делится между количеством процессов, то делим поровну и кайфуем
    если не делится то делаем следующее:
    M - числа
    N - процессы
    первым N - 1 процессам достается n = [M / N] - с округлением вниз
    N- ому процессу должно достаться n = M - (N - 1) * [M / N]
    !!!если количество процессов больше чем половина данных (N > [M / 2]) -> уменьшаем N до  [M / 2]
*/
int * divided_properly(int number_of_processes, int number_of_numbers){
    int * distribution = (int *) malloc(sizeof(int) * number_of_processes);
    if (number_of_numbers % number_of_processes == 0){ //если можно по братски поделить
        for (int i = 0; i < number_of_processes; i++) distribution[i] = number_of_numbers / number_of_processes;    
    }
    else { //воровской дележ
        int first = (int) floor((float) number_of_numbers / (float) number_of_processes);
        
        int last = number_of_numbers - ((number_of_processes - 1) * (int) (floor( (float) number_of_numbers / (float) number_of_processes)));
        for (int i = 0; i < number_of_processes; i++){ 
            if (i != number_of_processes - 1) distribution[i] = first;
            else distribution[i] = last;
        }
    }
    return distribution;
}