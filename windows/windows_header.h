#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <windows.h>




int check_argc(int argc);
int from_string_to_int(char * arg);
int lovit_cal(FILE * fp, int number_of_processes, int ipc_ctl);
int get_number_of_input(FILE * fp);
int check_data(int data_cnt);
void body(FILE * fp, int number_of_processes, int data_cnt, int ipc_ctl);
void check_and_reduce(int * number_of_processes, int arr_len);
void controller(int ipc_ctl, int number_of_processes, int arr_len, int * arr, int * sum);
int work_with_pipes(int number_of_process, int arr_len, int * arr, int * total_sum);
int * divided_properly(int number_of_processes, int number_of_numbers);
VOID InitializeSecurityAttr(LPSECURITY_ATTRIBUTES attr, SECURITY_DESCRIPTOR * sd);
