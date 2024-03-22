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
    else if (ipc_ctl == 1) {
        printf("Work with shared memory was choosen\n");
        work_with_shared_memory(number_of_processes, arr_len, arr, sum);
    }
}

int work_with_shared_memory(int number_of_process, int arr_len, int * arr, int * total_sum){
    int * distribution = divided_properly(number_of_process, arr_len);
    int * tmp_arrs[number_of_process];
    int keys[number_of_process];
    int index = 0,  res = 0;

    //атрибуты для создания процесса
    PROCESS_INFORMATION pi[number_of_process];
    STARTUPINFO si[number_of_process];

    for (int i = 0; i < number_of_process; i++){
        GetStartupInfo(&si[i]);
        keys[i] = 10 + i; // чтобы кайф был
        
        char key_buf[10];
        sprintf(key_buf, "%d", keys[i]);

        HANDLE hfile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 
                                        0, sizeof(int) * distribution[i], key_buf);
        if (hfile == NULL){
            printf("Error while creating file mapping\n");
            return -1;
        }

        tmp_arrs[i] = MapViewOfFile(hfile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * distribution[i]);
        if (tmp_arrs[i] == NULL) {
            printf("Error while mapping a view of file\n");
            return -1;
        } 
        for (int j = 0; j < distribution[i]; j++){
            tmp_arrs[i][j] = arr[index];
            index++; 
        }

        char args[100];
        sprintf(args, "1 %d %d", distribution[i], keys[i]);

        if (CreateProcess("subproc.exe", args, NULL, NULL, TRUE, 0, NULL, NULL, &si[i], &pi[i]) == 0){
            printf("Error while creating a process\n");
            return -1;
        }
    }

    for (int i = 0; i < number_of_process; i++){
        DWORD dwres = WaitForSingleObject(pi[i].hProcess, INFINITE);
        if (dwres == WAIT_FAILED){
            printf("Error while waiting\n");
        }
        else {
            DWORD read;
            res += tmp_arrs[i][0];
            if (UnmapViewOfFile(tmp_arrs[i]) == FALSE){
                printf("Error while detaching memory in parent process\n");
                return -1;
            }
        }
    }
    *total_sum = res;
    free(distribution);
    free(arr);
}   

int work_with_pipes(int number_of_process, int arr_len, int * arr, int * total_sum){
    int * distribution = divided_properly(number_of_process, arr_len);
    int index = 0, sum = 0;

    //хендлы для общения 0 - чтение 1 - запись
    HANDLE from_parent_to_child[number_of_process][2];
    HANDLE from_child_to_parent[number_of_process][2]; 
    HANDLE parentStdout = GetStdHandle(STD_ERROR_HANDLE);
    HANDLE childStdIn =  GetStdHandle(STD_INPUT_HANDLE);

    //аттрибуты для создания канала
    SECURITY_ATTRIBUTES attr;
    SECURITY_DESCRIPTOR sd; 
    InitializeSecurityAttr(&attr, &sd);

    //атрибуты для создания процесса
    PROCESS_INFORMATION pi[number_of_process];
    STARTUPINFO si[number_of_process];
    
    for (int i = 0; i < number_of_process; i++){
        DWORD writen;
        GetStartupInfo(&si[i]);

        if (CreatePipe(&from_parent_to_child[i][0], &from_parent_to_child[i][1], &attr, 0) == FALSE || CreatePipe(&from_child_to_parent[i][0], &from_child_to_parent[i][1], &attr, 0) == FALSE){
            printf("Error while creating a pipe\n");
            return -1;
        }

        int tmp_buf[distribution[i]];

        for (int j = 0; j < distribution[i]; j++){
            tmp_buf[j] = arr[index];
            index++;
        }

        if (WriteFile(from_parent_to_child[i][1], tmp_buf, sizeof(int) * distribution[i], &writen, NULL) == FALSE || CloseHandle(from_parent_to_child[i][1]) == FALSE){
            printf("Error while writing in file\n");
            return -1;
        }

        
        if (SetStdHandle(STD_INPUT_HANDLE, from_parent_to_child[i][0]) == FALSE || SetStdHandle(STD_ERROR_HANDLE, from_child_to_parent[i][1]) == FALSE){
            printf("Error while setting std handle\n");
            return -1;
        }
        
        char buf[20];
        sprintf(buf, "0 %d", distribution[i]); 
        if (CreateProcess("subproc.exe", buf, NULL, NULL, TRUE, 0, NULL, NULL, &si[i], &pi[i]) == 0){
            printf("Error whil creating process\n");
            return -1;
        }
    }

    for(int y = 0; y < number_of_process; y++){
        DWORD dwres = WaitForSingleObject(pi[y].hProcess, INFINITE);

        if (dwres == WAIT_FAILED){
            printf("Error while waiting\n");
        }
        else {
            int res;
            DWORD read;
            if (ReadFile(from_child_to_parent[y][0], &res, sizeof(int), &read, NULL) == FALSE || CloseHandle(from_child_to_parent[y][0]) == FALSE){
                printf("Error while reading file\n");
                return -1;
            }
            sum += res;
        }
    }
    if (SetStdHandle(STD_ERROR_HANDLE, parentStdout) == FALSE){
        printf("Error while setting std handle\n");
        return -1;
    } 
    *total_sum = sum;
    free(distribution);
    free(arr);
}

int InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR * sd)
{
    attr->nLength = sizeof(SECURITY_ATTRIBUTES);
    attr->bInheritHandle = TRUE; // Включаем наследование дескрипторов
    if (InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION) == FALSE) {
        printf("Error while init sec=ure descriptor\n");
        return -1;
    }
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