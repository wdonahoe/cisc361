#ifndef DISK
#define DISK

typedef struct access_data access_data;

void new_data(access_data* data, int arrival, int sector);
void free_access_data(access_data* data[], int len);
void show_data(access_data* data[], int len);
void check_arrivals(access_data* data[], int len, int read, int timer);
void show_metrics(int total_time, int max_time, int timer, int len);
int compare_data(const void* a, const void* b);
int setup_scheduler(const char* input_file, const char* scheduler, int sec_to_sec_seek, int end_to_end_seek);
void fcfs(access_data* data[], int len, int sec_to_sec_seek);
void sstn(access_data* data[], int len, int sec_to_sec_seek);
void scan(access_data* data[], int len, int sec_to_sec_seek);
void cscan(access_data* data[], int len, int sec_to_sec_seek);

#endif