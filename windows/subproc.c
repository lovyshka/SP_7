#include "windows_header.h"

int main(int argc, char * argv[]){
    int ctl, sum = 0;
    sscanf(argv[0], "%d", &ctl);
    if (ctl == 0){
        HANDLE    to_read = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE    to_write = GetStdHandle(STD_ERROR_HANDLE);
        
        if (to_read == NULL || to_write == NULL){
            printf("Errot while getting std handle in child proc\n");
            return -1;
        }

        int arr_len;

        DWORD reaten;

        sscanf(argv[1], "%d", &arr_len);
        int arr[arr_len];
        
        if (ReadFile(to_read, arr, sizeof(int) * arr_len, &reaten, NULL) == FALSE || CloseHandle(to_read) == FALSE){
            printf("Error while processing file in child process\n");
            return -1;
        }
        
        for (int i = 0; i < arr_len; i++) {
            if (arr[i] % 2 == 0) sum += arr[i];
        }
        if (WriteFile(to_write, &sum, sizeof(int), &reaten, NULL) == FALSE || CloseHandle(to_write) == FALSE){
            printf("Error while processing file in child process\n");
            return -1;
        }
    }
    if (ctl == 1){
        int arr_len, key;
        sscanf(argv[1], "%d", &arr_len);
        HANDLE hread = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, argv[2]);

        if (hread == NULL){
            printf("Error while opening handle in child process\n");
            return -1;
        }
    
        int * arr = MapViewOfFile(hread, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * arr_len);

        if (arr == NULL){
            printf("Error while mapping handle in child process\n");
            return -1;
        }
        for (int i = 0; i < arr_len; i++) if (arr[i] % 2 == 0) sum += arr[i];
        arr[0] = sum;
        if (UnmapViewOfFile(arr) == FALSE){
            printf("Error while detaching memory in child process\n");
            return -1;
        }
    }
    return 0;
}