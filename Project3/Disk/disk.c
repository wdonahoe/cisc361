#include "disk.h"
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#define TOTAL_BLOCKS 10000

struct access_data{
	int sector;
	int arrival_time;
	int checked;
	int processed;
};

void new_data(access_data* data, int arrival_time, int sector){
	data->sector = sector;
	data->arrival_time = arrival_time;
	data->checked = 0;
	data->processed = 0;
}

void free_access_data(access_data* data[], int len){
	int i = 0;
	while (i < len){
		free(data[i]);
		i++;
	}
}

void show_data(access_data* data[], int len){
	int i = 0;
	while (i < len){
		printf("data[%d]: arrival_time: %d, sector: %d\n", i, data[i]->arrival_time, data[i]->sector);
		i++;
	}
}

void check_arrivals(access_data* data[], int len, int read, int timer){
	while (read < len){
		if (data[read]->arrival_time <= timer && data[read]->checked == 0){
			printf("Disk read arrived at time %d for sector %d.\n",data[read]->arrival_time, data[read]->sector);
			data[read]->checked++;
		}
		read++;
	}
}

void show_metrics(int total_time, int max_time, int timer, int len){
	double avg_seek = (double)max_time / (double)len;
	double disk_util = (double)len / timer;
	printf("Maximum seek time: %d\n", max_time);
	printf("Avg Seek time: %f\n", avg_seek);
	printf("Disk utilization: %f\n", disk_util);
}

void fcfs(access_data* data[], int len, int sec_to_sec_seek){
	int i; int total_mvmnt; int timer; int head; int max_mvmnt;
	head = i = timer = total_mvmnt = max_mvmnt = 0;
	while (i < len){
		check_arrivals(data, len, i, timer);
		int mvmnt;
		if (data[i]->arrival_time <= timer){
			printf("Disk read processed at time %d for sector %d.\n", timer, data[i]->sector);
			mvmnt = abs(head - data[i]->sector);
			max_mvmnt = (mvmnt > max_mvmnt) ? mvmnt : max_mvmnt;
			total_mvmnt += mvmnt;
			head = data[i]->sector;
			timer += mvmnt * sec_to_sec_seek;
			i++;
		}
		else {
			timer = data[i]->arrival_time;
		}
	}
	show_metrics(total_mvmnt * sec_to_sec_seek, max_mvmnt * sec_to_sec_seek, timer, len);
}

void sstn(access_data* data[], int len, int sec_to_sec_seek){
	int head; int count; int movement; int read; int total_mvmnt; int timer; int max_mvmnt;
	head = count = movement = read = total_mvmnt = timer = max_mvmnt = 0;
	while (count < len){
		int i = 0;
		int closest = INT_MAX;

		check_arrivals(data, len, 0, timer);		
		while (i < len){
			if (data[i]->arrival_time <= timer){
				if (data[i]->processed == 0 && abs(data[i]->sector - head) < closest){
					closest = abs(data[i]->sector - head);
					read = i;
				}
			}
			i++;		
		}
		if (closest == INT_MAX){
			int j = 0;
			while (j < len){
				if (data[j]->processed == 0){
					if (data[j]->arrival_time < closest){
						closest = data[j]->arrival_time;
						read = j;
					}
				}
				j++;
			}
			timer = closest;
		}
		printf("Disk read processed at time %d for sector %d.\n", timer, data[read]->sector);
		movement = abs(data[read]->sector - head);
		max_mvmnt = (movement > max_mvmnt) ? movement : max_mvmnt;
		data[read]->processed++;
		total_mvmnt += movement;
		head = data[read]->sector;
		timer += movement * sec_to_sec_seek;
		count++;
	}
	show_metrics(total_mvmnt * sec_to_sec_seek, max_mvmnt * sec_to_sec_seek, timer, len);
}

int compare_data(const void* a, const void* b){
	return (*(access_data**)a)->sector - (*(access_data**)b)->sector;
}

void scan(access_data* data[], int len, int sec_to_sec_seek){
	int i; int dir; int cycled; int head; int count; int movement; int total_mvmnt; int timer; int max_mvmnt;
	i = dir = cycled = head = count = movement = total_mvmnt = timer = max_mvmnt = 0;

	qsort(data, len, sizeof(access_data*), compare_data);
	check_arrivals(data, len, i, timer);

	while (i >= 0 && i <= len && count < len){
		if (data[i]->arrival_time <= timer){
			if (data[i]->processed == 0){
				printf("Disk read processed at time %d for sector %d\n", timer, data[i]->sector);
				movement = abs(data[i]->sector - head);
				max_mvmnt = (movement > max_mvmnt) ? movement : max_mvmnt;
				data[i]->processed++;
				total_mvmnt += movement;
				head = data[i]->sector;
				timer += movement * sec_to_sec_seek;
				count++;
			}
		}
		else {
			timer += sec_to_sec_seek;
			movement++;
			total_mvmnt++;
		}

		if (i == len - 1){
			dir++;
			cycled = 1;
		}
		else if (i == 0 && cycled == 1){
			dir--;
		}
		i = (dir == 0) ? i + 1 : i - 1;
	}
	show_metrics(total_mvmnt * sec_to_sec_seek, max_mvmnt * sec_to_sec_seek, timer, len);
}

void cscan(access_data* data[], int len, int sec_to_sec_seek){
	int i; int cycled; int head; int count; int movement; int total_mvmnt; int timer; int max_mvmnt;
	i = cycled = head = count = movement = total_mvmnt = timer = max_mvmnt = 0;

	qsort(data, len, sizeof(access_data*), compare_data);
	check_arrivals(data, len, i, timer);

	while (i < len && count < len - 1){
		if (data[i]->arrival_time <= timer){
			if (data[i]->processed == 0){
				printf("Disk read processed at time %d for sector %d\n", timer, data[i]->sector);
				movement = abs(data[i]->sector - head);
				max_mvmnt = (movement > max_mvmnt) ? movement : max_mvmnt;
				data[i]->processed++;
				total_mvmnt += movement;
				head = data[i]->sector;
				timer += movement * sec_to_sec_seek;
				count++;
			}
		}
		else {
			timer += sec_to_sec_seek;
			movement++;
			total_mvmnt++;
		}

		if (i == len - 1){
			i = 0;
		}
		i++;
	}
	show_metrics(total_mvmnt * sec_to_sec_seek, max_mvmnt * sec_to_sec_seek, timer, len);
}

int setup_scheduler(const char* input_file, const char* scheduler, int sec_to_sec_seek, int end_to_end_seek){
	char** tokens = read_input(input_file);
	int len = array_len(tokens);
	access_data* data[len];
	char* tok;
	char* line;

	int i = 0;
	while (i < len){
		line = tokens[i];
		tok = strtok(line," ");

		int arrival_time = atoi((const char*)tok);
    	tok = strtok(NULL," ");
    	int sector = atoi((const char*)tok);

		access_data* a = malloc(sizeof(access_data));
		new_data(a, arrival_time, sector);
		data[i] = a;

		i++;
	}

	//show_data(data, len);

	if (strcmp(scheduler, "FCFS") == 0){
		fcfs(data, len, sec_to_sec_seek);
	}
	else if (strcmp(scheduler, "SSTN") == 0){
		sstn(data, len, sec_to_sec_seek);
	}
	else if (strcmp(scheduler, "SCAN") == 0){
		scan(data, len, sec_to_sec_seek);
	}
	else if (strcmp(scheduler, "CSCAN") == 0){
		cscan(data, len, sec_to_sec_seek);
	}
	else{
		fprintf(stderr, "%s\n","Invalid scheduling algorithm.");
		exit(EXIT_FAILURE);
	}

	free_access_data(data, len);
	free(tokens);

	return 0;
}

/**
	Read input, print usage message when given fewer than four arguments.
	Otherwise setup scheduler. 
*/
int main(int argc, char* argv[]){

	if (argc < 5){
		printf("discksched: missing arguments\n\nusage: ./disk <input_file> <scheduling_algorithm> <sector-to-sector_seek_time> <end-to-end_seek_time>\n\n");
		printf("Supported schedulers:\n\n");
		printf("   1) FCFS \n");
		printf("   2) SSTN \n");
		printf("   3) SCAN \n");
		printf("   4) C-SCAN \n\n");
	}
	else if (argc == 5) {
		const char* input_file = (const char*)argv[1];
		const char* algorithm = (const char*)argv[2];
		int sec_to_sec_seek = atoi((const char*)argv[3]);
		int end_to_end_seek = atoi((const char*)argv[4]);

		setup_scheduler(input_file, algorithm, sec_to_sec_seek, end_to_end_seek);
	}
	else {
		fprintf(stderr, "error: too many arguments to disksched.\n");
	}

	return 0;
}