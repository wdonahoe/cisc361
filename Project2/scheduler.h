#ifndef SCHEDULER
#define SCHEDULER

typedef struct proc_data proc_data;
typedef struct bursts bursts;
typedef struct waits waits;

void new_data(proc_data* data, char* tokens, int priority, int adj);
void get_bursts(bursts* bs, char** tokens, int len);
void get_waits(waits* ws, char** tokens, int len);

#endif