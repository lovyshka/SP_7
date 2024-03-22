#include <windows.h>
#include <stdio.h>

VOID InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR * sd);

int main(){

    int arr[] = {1, 2, 3, 4, 5, 6};
    int arr_len = sizeof(arr) / sizeof(arr[0]);

    int key = 7;
    char key_buf[] = "7";

    HANDLE hfile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 
                                        0, sizeof(int) * arr_len, key_buf);
    int * tmp = MapViewOfFile(hfile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int) * arr_len);

    for (int i = 0; i < arr_len; i++) tmp[i] = i + 1;

    PROCESS_INFORMATION pi;
    STARTUPINFO si;
    GetStartupInfo(&si);

    char args[100];
    sprintf(args, "%d %d", arr_len, key);
    CreateProcess("subproc.exe", args, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);

    WaitForSingleObject(pi.hProcess, INFINITE);
    printf("sum = %d\n", tmp[0]);
    UnmapViewOfFile(arr);
    return 0;
}
