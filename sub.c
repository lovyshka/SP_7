#include <windows.h>
#include <stdio.h>

int main(int argc, char *argv[]){
    int len;
    sscanf(argv[0], "%d", &len);
    printf("len = %d\n", len);
    int arr[len];
    HANDLE read = GetStdHandle(STD_INPUT_HANDLE);
    char buf[10];
    DWORD reaten;
    ReadFile(read, arr, sizeof(int) * len, &reaten, NULL);
    for (int i = 0; i < len; i++) printf("%d\n", arr[i]);
    CloseHandle(read);
    printf("%s\n", buf);
    return 0;
}
