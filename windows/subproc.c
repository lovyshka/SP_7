#include "windows_header.h"

int main(int argc, char * argv[]){
    int ctl, sum = 0;
    sscanf(argv[0], "%d", &ctl);
    if (ctl == 0){
        HANDLE    to_read = GetStdHandle(STD_INPUT_HANDLE);
        HANDLE    to_write = GetStdHandle(STD_ERROR_HANDLE);

        int arr_len;

        DWORD reaten;

        sscanf(argv[1], "%d", &arr_len);
        int arr[arr_len];
        
        ReadFile(to_read, arr, sizeof(int) * arr_len, &reaten, NULL);
        CloseHandle(to_read);
        
        for (int i = 0; i < arr_len; i++) {
            if (arr[i] % 2 == 0) sum += arr[i];
        }
        WriteFile(to_write, &sum, sizeof(int), &reaten, NULL);
        CloseHandle(to_write);
    }
    if (ctl == 1){
        int arr_len, key;
        sscanf(argv[1], "%d", &arr_len);
        HANDLE hread = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, argv[2]);
    
        int * arr = MapViewOfFile(hread, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * arr_len);

        for (int i = 0; i < arr_len; i++) if (arr[i] % 2 == 0) sum += arr[i];
        arr[0] = sum;
        UnmapViewOfFile(arr);
    }
    return 0;
}