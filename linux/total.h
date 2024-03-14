#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <errno.h>
#include <ctype.h>
#include <sys/ipc.h>
#include <sys/shm.h>


void body(FILE * fp, int number_of_processes, int data_cnt, int ipc_ctl);
void check_and_reduce(int * number_of_processes, int arr_len);
int check_argc(int argc);
int check_data(int data_cnt);
int lovit_cal(FILE * fp, int number_of_processes, int ipc_ctl);
int * divided_properly(int number_of_processes, int number_of_numbers);
int from_string_to_int(char * arg);

int get_number_of_input(FILE * fp);
int work_with_pipes(int number_of_process, int arr_len, int * arr, int * total_sum);
int get_total_sum(int number_of_process, int (* fd_from_ch_to_p)[2]);
int work_with_shared_memory(int number_of_process, int arr_len, int * arr, int * total_sum);
void controller(int ipc_ctl, int number_of_processes, int arr_len, int * arr, int * sum);