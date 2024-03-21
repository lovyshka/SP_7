#include "windows_header.h"

int main(int argc, char * argv[]){
    HANDLE    to_read = GetStdHandle(STD_INPUT_HANDLE);
    HANDLE    to_write = GetStdHandle(STD_ERROR_HANDLE);

    int arr_len, sum = 0;

    DWORD reaten;

    sscanf(argv[0], "%d", &arr_len);
    int arr[arr_len];
    
    ReadFile(to_read, arr, sizeof(int) * arr_len, &reaten, NULL);
    CloseHandle(to_read);
    
    for (int i = 0; i < arr_len; i++) {
        if (arr[i] % 2 == 0) sum += arr[i];
    }
    WriteFile(to_write, &sum, sizeof(int), &reaten, NULL);
    CloseHandle(to_write);
    return 0;
}