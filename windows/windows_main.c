#include "windows_header.h"


int main(int argc, char *argv[]){   
    if (check_argc(argc) == 1) return -1; //checek argc
    
    int number_of_processes = from_string_to_int(argv[2]); 
    int ipc_ctl = from_string_to_int(argv[3]);
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
    }
    // else if (ipc_ctl == 1) {
    //     printf("Work with shared memory was choosen\n");
    //     work_with_shared_memory(number_of_processes, arr_len, arr, sum);
    // }
}

int work_with_pipes(int number_of_process, int arr_len, int * arr, int * total_sum){
    int * distribution = divided_properly(number_of_process, arr_len);
    HANDLE parent_write_pipe, child_read_pipe;
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
    int index, sum = 0;
    DWORD reaten;
    PROCESS_INFORMATION pi;
    STARTUPINFO si;

    SECURITY_ATTRIBUTES attr;
    SECURITY_DESCRIPTOR sd;
    putchar('!');
    InitializeSecurityAttr(&attr, &sd);
    SetStdHandle(hStdin, child_read_pipe);
    CreatePipe(&child_read_pipe, &parent_write_pipe, &attr, 0);
    WriteFile(parent_write_pipe, arr, sizeof(int) * arr_len, &reaten, NULL);
    CloseHandle(parent_write_pipe);
    //ctl, handle read, handle write
    LPSTR args = (LPSTR)malloc(100);
    sprintf(args, "%p %d", child_read_pipe, arr_len);
    CreateProcess(
        "./subproc.exe",
        args,
        NULL,
        NULL,
        TRUE,
        0,
        NULL,
        NULL,
        &si,
        &pi);


    // SetStdHandle(hStdin, parent_read_pipe);


    // CloseHandle(childe_read_pipe);
    SetStdHandle(STD_INPUT_HANDLE, hStdin);
}

VOID InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR * sd)
{
    attr->nLength = sizeof(SECURITY_ATTRIBUTES);
    attr->bInheritHandle = TRUE; // Включаем наследование дескрипторов
    InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    attr->lpSecurityDescriptor = sd;
}


void check_and_reduce(int * number_of_processes, int arr_len){
    if (*number_of_processes > ((float) arr_len) / 2.0){
        *number_of_processes = (int)floor(arr_len / 2);
        printf("number of processes were reduced to %d\n", *number_of_processes);
    }
}

int check_data(int data_cnt){
    int flag = 0;
    if (data_cnt <= 1){
        flag = 1;
        printf("Empty file given or there are less then 2 numbers\n");
    }
    return flag;
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

int check_argc(int argc){
    int flag = 0;
    if (argc != 4){
        flag = 1;
        printf("Invalid number of arguments: see usage\n ./a.out [input file] [nuber of child process] [0 - pipes, 1 - shared mamory]\n");
    }
    return flag;
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