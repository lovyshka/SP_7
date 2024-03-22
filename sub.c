#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[]){
    int arr_len, sum = 0;
    sscanf(argv[0], "%d", &arr_len);
    printf("argc = %d\narr_len = %d\n",argc,  arr_len);
    HANDLE hread = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, argv[1]);

    int * arr = MapViewOfFile(hread, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * arr_len);

    for (int i = 0; i < arr_len; i++) if (arr[i] % 2 == 0) sum += arr[i];
    
    arr[0] = sum;
    UnmapViewOfFile(arr);
    return 0;
}
