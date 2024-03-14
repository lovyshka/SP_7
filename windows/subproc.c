#include "windows_header.h"

int main(int argc, char * argv[]){
    HANDLE hread;
    int arr_len;
    DWORD reaten;
    sscanf(argv[1], "%p %d", &hread, &arr_len);
    printf("%d", arr_len);
    int * arr;
    ReadFile(hread, arr, sizeof(int) * arr_len, &reaten, NULL);
    for (int i = 0; i < arr_len; i++){
        printf("%d\n", arr[i]);
    }
    return 0;
}