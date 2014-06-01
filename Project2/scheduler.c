#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <math.h>
#include "scheduler.h"

#define PID 0
#define TAT 1
#define IO 2
#define WT 3
#define RT 4
#define METRICS 5

/**

	Contains number of bursts and array of burst times.
	@member start_time: start time of the process.
	@member nbursts: total number of bursts.
	@member data: array of durations for each burst period.
	
*/
struct bursts {
	int nbursts;
	int* data;
};

/**

	Contains number of waits and array of wait times.
	@member nwaits: total number of waits.
	@member data: array of durations for each wait period.

*/
struct waits {
	int nwaits;
	int* data;
};

/**

	Contains all the data for a single process.
	@member start_time: start time of the process.
	@member time_in_queue: total time spent in queue.
	@member total_time: total time the process has been running.
	@member bursts: contains burst data.
	@member waits: contains wait data.

*/
struct proc_data {
	int start_time;
	int priority;
	int pid;
	int ready_time;
	int burstnum;
	int cpu_time;
	int wait_time;
	int sleep_until;
	int first_time;
	int seen;
	int examined;
	int finish;
	bursts* bursts;
	waits* waits;
};

/**
	
	Creates a new proc_data.
	@param line: a string containing the running information for a process.
				 Format: Ta N, C1, D1, C2, ... , Cn-1, Dn, Cn
				 	Ta = Arrival time of the process.
				 	N = #CPU bursts for the process.
				 	Ci = Duration of CPU burst i.
				 	Di = Duration of i/o wait time.

*/
void new_data(proc_data* data, char* line, int priority, int adj){
	char** tokens = tokenize(line, " ,");
	int start_time = atoi((const char*)tokens[0]);
	int nbursts = atoi((const char*)tokens[1]);
	bursts* bs = malloc(sizeof(bursts*));
	waits* ws = malloc(sizeof(waits*));
		
	data->start_time = start_time - adj;
	data->priority = priority;
	data->pid = priority;
	data->ready_time = 0;
	data->burstnum = 0;
	data->cpu_time = 0;
	data->wait_time = 0;
	data->sleep_until = 0;
	data->first_time = 0;
	data->seen = 0;
	data->examined = 0;
	data->finish = 0;

	get_bursts(bs, tokens, nbursts);
	get_waits(ws, tokens, nbursts - 1);

	data->bursts = bs;
	data->waits = ws;

	free(tokens);

}

void get_waits(waits* ws, char** tokens, int nwaits){
	int* data = malloc(sizeof(int) * nwaits);
	char** nums = tokens + 3;
	int i = 0;

	while (i < nwaits){
		data[i] = atoi((const char*)*nums);
		i++;
		nums = nums + 2;
	}

	ws->nwaits = nwaits;
	ws->data = data;
}

void get_bursts(bursts* bs, char** tokens, int nbursts){
	int* data = malloc(sizeof(int) * nbursts);
	char** nums = tokens + 2;
	int i = 0;

	while (i < nbursts){
		data[i] = atoi((const char*)*nums);
		i++;
		nums = nums + 2;
	}

	bs->nbursts = nbursts;
    bs->data = data;
}

void free_proc_data(proc_data* data[], int len){
	int i = 0;
	while (i < len){
		free(data[i]->bursts);
		free(data[i]->waits);
		free(data[i]);
		i++;
	}
}

int qsum_array(int A[], int size, int cs, int q){
	int sum = 0;
	int i = 0;
	int divide;
	while (i < size){
		divide = 1 + ((A[i] - 1) / q);
		sum += A[i++] + (divide * cs);
	}

	return sum;
}

int get_runtime(proc_data* data[], int nprocs, int cs, int quantum){
	int i = 0;
	int total = 0;

	while (i < nprocs){
		bursts* bs = data[i]->bursts;
		waits* ws = data[i]->waits;
		if (quantum == 0){
			total += sum_array(bs->data, bs->nbursts) + sum_array(ws->data, ws->nwaits) + (bs->nbursts * cs);
		}
		else {
			total += qsum_array(bs->data, bs->nbursts, cs, quantum) + sum_array(ws->data, ws->nwaits) + (bs->nbursts * cs);
		}
		i++;
	}
	return total;
}

int ready(proc_data* data, int timer){
	return timer >= data->start_time && data->burstnum < data->bursts->nbursts;
}

void reset_priorities(proc_data* data[], int curr_index, int nprocs){
	int i = 0;
	printf("==============\n");
	while (i < nprocs){
		if (data[i]->priority != 1 && data[i]->bursts->nbursts != data[i]->burstnum)
			data[i]->priority--;
		i++;
	}
	data[curr_index]->priority = nprocs;

	i = 0;
	while (i < nprocs){
		//printf("P%d new priority: %d\n",data[i]->pid, data[i]->priority);
		i++;
	}
}

void show_analysis(int* analysis, int timer){
	printf("Process %d terminated at t = %d.\n",analysis[PID], timer);
	printf("TAT: %d\nRT: %d\nWT : %d\nIO : %d\n",analysis[TAT], analysis[RT], analysis[WT], analysis[IO]);
}

void analyze_proc(proc_data* data, int* analysis){

	int tat = data->finish - data->start_time;
	int rt = data->first_time - data->start_time; 
	int wt = tat - data->cpu_time;
	int io = data->wait_time;

	analysis[PID] = data->pid;
	analysis[TAT] = tat;
	analysis[RT] = rt;
	analysis[WT] = wt;
	analysis[IO] = io;
}

void analyze_all(proc_data* data[], int nprocs){
	int* analysis = malloc(sizeof(int) * METRICS);
	int i = 0;
	int max_tat = 0;
	int max_wait = 0;
	int max_rt = 0;
	double cpu_util = 0;

	while (i < nprocs){
		analyze_proc(data[i], analysis);
		if (analysis[TAT] > max_tat) max_tat = analysis[TAT];
		if (analysis[RT] > max_rt) max_rt = analysis[RT];
		if (analysis[WT] > max_wait) max_wait = analysis[WT];
		cpu_util += (double)data[i]->cpu_time / (double)(data[i]->finish - data[i]->first_time);
		i++;
	}
	cpu_util = 100 * (cpu_util / nprocs);
	printf("MAX RT: %d\n",max_rt);
	printf("MAX TAT: %d\n",max_tat);
	printf("MAX WAIT: %d\n",max_wait);
	printf("CPU UTIL: %f%c\n",cpu_util, 0x25);

	free(analysis);
}


void run_fcfs(proc_data* data[], char* const* params, int nparams, int nprocs){
	int timer = 0;
	int cs = atoi((const char*)params[0]) << 1;
	proc_data* curr;
	int total_runtime = get_runtime(data, nprocs, cs, 0);
	
	while (timer < total_runtime){
		int i = 0;
		int p = INT_MAX;
		int curr_index;
		while (i < nprocs){
			if (data[i]->priority < p && ready(data[i], timer)){
				p = data[i]->priority;
				curr = data[i];
				curr_index = i;
			}
			i++;
		}
		if (p == INT_MAX){
			timer++;
		}
		else {
			timer += run(curr, cs, timer);

			if (curr->burstnum == curr->bursts->nbursts && curr->examined == 0){
				int* a = malloc(sizeof(int) * METRICS);
				curr->finish = timer;
				printf("cpu time: %d\n",curr->cpu_time);
				analyze_proc(curr, a);
				show_analysis(a, timer);
				curr->examined = 1;
				free(a);
			}
			printf("total time: %d\n",timer);
			reset_priorities(data, curr_index, nprocs);	
		}
	}
	analyze_all(data, nprocs);
}

void run_rr(proc_data* data[], char* const* params, int nparams, int nprocs){
	int timer = 0;
	int cs = atoi((const char*)params[0]) << 1;
	int quantum = atoi((const char*)params[1]);
	proc_data* curr;
	int total_runtime = get_runtime(data, nprocs, cs, quantum);
	
	while (timer < total_runtime){
		int i = 0;
		int p = INT_MAX;
		int curr_index;
		while (i < nprocs){
			if (data[i]->priority < p && ready(data[i], timer)){
				p = data[i]->priority;
				curr = data[i];
				curr_index = i;
			}
			i++;
		}
		if (p == INT_MAX){
			timer++;
		}
		else {
			timer += run_quanta(curr, cs, timer, &quantum);

			if (curr->burstnum == curr->bursts->nbursts && curr->examined == 0){
				int* a = malloc(sizeof(int) * METRICS);
				curr->finish = timer;
				printf("finish time: %d\n",timer);
				printf("cpu time: %d\n",curr->cpu_time);
				analyze_proc(curr, a);
				show_analysis(a, timer);
				curr->examined = 1;
				free(a);
			}
			printf("total time: %d\n",timer);
			reset_priorities(data, curr_index, nprocs);	
		}
	}
	analyze_all(data, nprocs);
}

int run(proc_data* data, int cs, int timer){
	int ret = data->bursts->data[data->burstnum];
	int wait = data->waits->data[data->burstnum];
	int t = 0;

	if (data->seen == 0){
		data->first_time = timer + cs;
		data->seen = 1;
		printf("Process %d has arrived at t = %d, waiting for %d clock cycles.\n",data->pid, data->start_time, timer - data->start_time);
	}

	if (data->sleep_until <= timer){
		data->cpu_time += ret;
		data->sleep_until = 0;

		printf("Process %d runs for %d clock cycles.\n", data->pid, ret);

		if (data->waits->nwaits != 0 && data->waits->nwaits != data->burstnum){
			printf("Process %d starts waiting for %d clock cycles\n",data->pid, wait);
			data->wait_time += wait;
			data->sleep_until = wait + timer;
			t = cs;
		}
		data->burstnum++;

		return ret + cs + t;
	}
	else{
		printf("Process %d still waiting for %d cycles.\n",data->pid, data->sleep_until - timer);
		return 1;
	}
}

int run_quanta(proc_data* data, int cs, int timer, int* quantum){
	int ret = data->bursts->data[data->burstnum];
	int wait = data->waits->data[data->burstnum];
	int t = 0;

	if (data->seen == 0){
		data->first_time = timer + cs;
		data->seen = 1;
		printf("Process %d has arrived.\n", data->pid);
	}

	if (data->sleep_until <= timer){

		if (ret - *quantum > 0){
			ret = *quantum;
			data->cpu_time += ret;
			data->bursts->data[data->burstnum] -= *quantum;
			data->sleep_until = 0;
			printf("Process %d runs for %d clock cycles.\n", data->pid, ret);
			printf("Process %d was preempted.\n", data->pid);
		}
		else {
			printf("Process %d runs for %d clock cycles.\n",data->pid, ret);

			if (data->waits->nwaits != 0 && data->waits->nwaits != data->burstnum){
				printf("Process %d starts waiting for %d clock cycles\n",data->pid, wait);
				data->wait_time += wait;
				data->sleep_until = wait + timer;
				t = cs;
			}
			data->burstnum++;

		}
		return ret + cs + t;
	}
	else{
		printf("Process %d still waiting for %d cycles.\n",data->pid, data->sleep_until - timer);
		return 1;
	}
}

void show(proc_data* data[], int len){
	int i = 0;
	int j = 0;
	int k = 0;

	while (i < len){
		printf("P%d \n",data[i]->pid);
		printf("priority: %d\n", data[i]->priority);
		printf("start time: %d\n",data[i]->start_time);
		printf("bursts:");
		while (j < data[i]->bursts->nbursts){
			printf(" %d",data[i]->bursts->data[j]);
			j++;
		}
		j = 0;
		printf("\nwaits:");
		while (k < data[i]->waits->nwaits){
			printf(" %d",data[i]->waits->data[k]);
			k++;
		}
		k = 0;
		i++;
		printf("\n%s\n","===========");
	}

}

void setup_scheduler(const char* input_file, const char* scheduler, char* const* params, int nparams){
	char** tokens = read_input(input_file);
	int len = s_array_len(tokens);
	proc_data* data[len];
	int adj;
	
	proc_data* p = malloc(sizeof(proc_data));
	new_data(p, tokens[0], 1, 0);
	adj = (p->start_time != 0) ? p->start_time : 0;
	data[0] = p;
	data[0]->start_time -= adj;

	int i = 1;
	while (i < len){
		proc_data* p = malloc(sizeof(proc_data));
		new_data(p, tokens[i], i + 1, adj);
		data[i] = p;
		i++;
	}
	//show(data, len);		
	
	if (strcmp(scheduler, "FCFS") == 0){
		run_fcfs(data, params, nparams, i);
	}
	else if (strcmp(scheduler, "RR") == 0){
		run_rr(data, params, nparams, i);
	}
	else {
		fprintf(stderr, "%s\n", "No such scheduling algorithm, try again.");
	}
	
	free_proc_data(data, len);
	
	free(tokens);
	
}

int main(int argc, char* argv[]){
	const char* input_file;
	const char* scheduler;
	char* const* params;
	int nparams;

	if (argc < 3){
		printf("scheduler: missing arguments\n\n usage: ./scheduler <input_file> <algorithm> <parameters>\n\n");
		printf(" supported schedulers:\n");
		printf("   1) FCFS (non-preemptive) Parameters: Cost of half C.S.\n");
		printf("   2) RR (preemptive) Parameters: Quantum, Cost of half C.S.\n\n");
	}
	else {
		setup_scheduler((const char*)argv[1], (const char*)argv[2], (char* const*)(argv + 3), argc - 3);
	}

	return 0;
}
