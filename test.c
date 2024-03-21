#include <windows.h>
#include <stdio.h>

VOID InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR * sd);

int main(){
    HANDLE parent_write_pipe , child_read_pipe;
    SECURITY_ATTRIBUTES attr;
    SECURITY_DESCRIPTOR sd;

    int arr[] = {1, 2, 3, 4, 5, 6};
    int arr_len = sizeof(arr) / sizeof(arr[0]);
    DWORD writen;
    InitializeSecurityAttr (&attr, &sd); // Используем собственную функцию
    
    CreatePipe(&child_read_pipe, &parent_write_pipe , &attr, 0);
    
    WriteFile(parent_write_pipe, arr, sizeof(int) * arr_len, &writen, NULL);
    CloseHandle(parent_write_pipe);

    HANDLE StdIn = GetStdHandle(STD_INPUT_HANDLE );
    SetStdHandle(STD_INPUT_HANDLE , child_read_pipe); // Заменяем дескриптор
    
    
    STARTUPINFO si;
    GetStartupInfo(&si);
    PROCESS_INFORMATION pi;
    char buf[20];
    sprintf(buf, "%d", arr_len);
    CreateProcess("subproc.exe", buf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi);
    

    WaitForSingleObject(pi.hProcess, INFINITE);
   
    SetStdHandle(STD_INPUT_HANDLE , StdIn); // Возвращаем консольный ввод
    return 0;
}

VOID InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR * sd) {
    attr->nLength = sizeof(SECURITY_ATTRIBUTES);
    attr->bInheritHandle = TRUE; // Включаем наследование дескрипторов
    InitializeSecurityDescriptor(sd, SECURITY_DESCRIPTOR_REVISION);
    attr->lpSecurityDescriptor = sd;
}