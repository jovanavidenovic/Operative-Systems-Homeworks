#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/resource.h>
#include <math.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <stdbool.h>
#include <wait.h>
/*
find 
	Linux kernel version
	version of the gcc complier for compiling the kernel
*/
void sys(char* path){
    strcat(path, "/version");
    FILE* file = fopen(path, "r");

    char* str1 = malloc(100*sizeof(char));
    char* str2 = malloc(100*sizeof(char));
    char* str3 = malloc(100*sizeof(char));

    char* version_linux = malloc(100*sizeof(char));
    char* version_gcc = malloc(100*sizeof(char));

 	fscanf(file, "%s %s %s", str1, str2, version_linux);
	fscanf(file, "%s %s %s %s", str1, str2, str3, version_gcc);

    printf("Linux: %s\ngcc: %s\n", version_linux, version_gcc);

 	free(str1); free(str2); free(str3);
    free(version_linux); free(version_gcc);
    fclose(file);
}

/*	
find 
	Linux kernel version
	version of the gcc complier for compiling the kernel
	first swap partition
	number of kernel modules
*/
void sysext(char* path){

	char* path2 = malloc(strlen(path)*sizeof(char));
	memcpy(path2, path, strlen(path));

	char* path3 = malloc(strlen(path)*sizeof(char));
	memcpy(path3, path, strlen(path));

	// caling the basic function sys
	sys(path);	

	// first swap partition
	strcat(path2, "/swaps");
	FILE* file2 = fopen(path2, "r");
	char* line = malloc(1024*sizeof(char));
	char* first_swap_partition = malloc(100*sizeof(char));
	size_t len = 0;
	getline(&line, &len, file2);
	fscanf(file2, "%s", first_swap_partition);

	printf("Swap: %s\n", first_swap_partition);

	// number of kernel modules	
	strcat(path3, "/modules");
	FILE* file3 = fopen(path3, "r");
	int number_modules = 0;
	while(getline(&line, &len, file3) != -1)
		number_modules ++;

	printf("Modules: %d\n", number_modules);

	free(first_swap_partition);
	free(path2); free(path3); free(line);
	fclose(file2);
	fclose(file3);
}

/*
	using syscalls get
1	Uid, EUid, Gid, EGid, Cwd and process priority,
2   the path to the directory in the proc file system with process information 
 	   and the accessibility of this directory,
3   name, release and version of the operating system,
4   information about the computer and its name,
5   time zone information
6   the maximum possible processor time to run the process.
*/
void me(){
	// 1
	uid_t uid = getuid();
	printf("Uid: %d\n", uid);

	uid_t euid = geteuid();
	printf("EUid: %d\n", euid);

	gid_t gid = getgid();
	printf("Gid: %d\n", gid);

	gid_t egid = getegid();
	printf("EGid: %d\n", egid);

	char* cwd = malloc(256*sizeof(char));
	getcwd(cwd, 256);
	printf("Cwd: %s\n", cwd);

	pid_t pid = getpid();
	int priority = getpriority(PRIO_PROCESS, pid);
	printf("Priority: %d\n", priority);

	free(cwd);

	// 2	
	printf("Process proc path: /proc/%d/\n", pid);
	char path_dir[(int)(100*sizeof(char))];
	sprintf(path_dir, "/proc/%d/", pid);

	if(access(path_dir, R_OK) == 0)
		printf("Process proc access: yes\n");
	else printf("Process proc access: no\n");

	// 3
	struct utsname utsname_data;
	uname(&utsname_data);
	printf("OS name: %s\n", utsname_data.sysname);
	printf("OS release: %s\n", utsname_data.release);
	printf("OS version: %s\n", utsname_data.version);

	// 4
	printf("Machine: %s\n", utsname_data.machine);
	printf("Node name: %s\n", utsname_data.nodename);

	// 5
	struct timeval *tv =  malloc(sizeof(struct timeval));
	struct timezone *tz = malloc(sizeof(struct timezone));
	gettimeofday(tv, tz);

	printf("Timezone: %d\n", tz->tz_dsttime);
	free(tv); free(tz);
	// 6
	struct rlimit *rlim = malloc(sizeof(struct rlimit));
	getrlimit(RLIMIT_CPU, rlim);
	printf("CPU limit: %ld\n", rlim->rlim_cur);
	free(rlim);
}

// check whether given string represents pid
bool ispid(char* str){
	for(int i = 0; i < strlen(str); i++){
		if(!(str[i] >= '0' && str[i] <= '9'))
			return false;
	}
	return true;
}

int cmpfunc (const void * a, const void * b) {
   return ( *(int*)a - *(int*)b );
}

void pids(char* path){
	DIR *dir = opendir(path);	
	struct dirent *content_dir = malloc(sizeof(struct dirent));

	int num_proc = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			num_proc ++;
		}
	}
	closedir(dir);
	int *res = malloc(num_proc*sizeof(int)); int idx = 0;
	dir = opendir(path);

	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			res[idx] = atoi(content_dir->d_name);
			idx ++;
		}
	}

	qsort(res, num_proc, sizeof(int), cmpfunc);

	for(int i = 0; i < num_proc; i++)
		printf("%d\n", res[i]);

	free(res);
	closedir(dir);
}

struct PID_Name {
	int pid;
	char name[1024];
};

char* get_process_name(char* path, int pid){
	char* name = malloc(1024 * sizeof(char));
	sprintf(name, "%s/%d/comm", path, pid);
	FILE* file = fopen(name, "r");
	fscanf(file, "%s", name);
	fclose(file);
	return name;
}

void names(char* path){
	DIR *dir = opendir(path);	
	struct dirent *content_dir = malloc(sizeof(struct dirent));

	int num_proc = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			num_proc ++;
		}
	}

	closedir(dir);
	dir = opendir(path);

	struct PID_Name ** processes = malloc(num_proc*sizeof(struct PID_Name*));
	for(int i = 0; i < num_proc; i++){
		processes[i] = malloc(sizeof(struct PID_Name));
	}

	int idx = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			processes[idx]->pid = atoi(content_dir->d_name);
			strcpy(processes[idx]->name, get_process_name(path, processes[idx]->pid));
			idx ++;
		}
	}
	
	for(int i = 0; i < num_proc; i++){
		int min_idx = i;
		for(int j = i + 1; j < num_proc; j++){
			if(strcmp(processes[min_idx]->name, processes[j]->name) > 0){
				min_idx = j;
			} else if (strcmp(processes[min_idx]->name, processes[j]->name) == 0 && 
				processes[min_idx]->pid > processes[j]->pid){
				min_idx = j;
			}
		}

		struct PID_Name * temp_proc = malloc(sizeof(struct PID_Name));
		memcpy(temp_proc, processes[i], sizeof(struct PID_Name));
		free(processes[i]); processes[i] = malloc(sizeof(struct PID_Name));
		memcpy(processes[i], processes[min_idx], sizeof(struct PID_Name));
		free(processes[min_idx]); processes[min_idx] = malloc(sizeof(struct PID_Name));
		memcpy(processes[min_idx], temp_proc, sizeof(struct PID_Name));
		free(temp_proc);
	}

	for(int i = 0; i < num_proc; i++){
		printf("%d %s\n", processes[i]->pid, processes[i]->name);
		free(processes[i]);
	}

	free(processes);
	closedir(dir);
}

struct ps {
	int pid;
	int ppid;
	char state[1024];
	char name[1024];
};

void get_process_ppid_state(char* path, int pid, int* ppid, char* state){
	char* file_name = malloc(256 * sizeof(char));
	sprintf(file_name, "%s/%d/status", path, pid);

	FILE* file = fopen(file_name, "r");
	char* line = malloc(1024*sizeof(char));
	size_t len = 0;
	getline(&line, &len, file);

	char* help_str = malloc(1024*sizeof(char));
	fscanf(file, "%s %s %s", help_str, state, help_str);	
		
	getline(&line, &len, file);
	getline(&line, &len, file);
	getline(&line, &len, file);	
	getline(&line, &len, file);	
	fscanf(file, "%s %d", help_str, ppid);
		
	free(file_name); fclose(file);	
}

void ps(char* path, int wanted_pid){

	DIR *dir = opendir(path);	
	struct dirent *content_dir = malloc(sizeof(struct dirent));

	int num_proc = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			num_proc ++;
		}
	}

	closedir(dir);
	dir = opendir(path);

	struct ps** processes = malloc(num_proc*sizeof(struct ps*));
	for(int i = 0; i < num_proc; i++){
		processes[i] = malloc(sizeof(struct ps));
	}

	int idx = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			processes[idx]->pid = atoi(content_dir->d_name);
			get_process_ppid_state(path, processes[idx]->pid, &(processes[idx]->ppid), processes[idx]->state);
			strcpy(processes[idx]->name, get_process_name(path, processes[idx]->pid));

			idx ++;
		}
	}
	
	for(int i = 0; i < num_proc; i++){
		int min_idx = i;
		for(int j = i + 1; j < num_proc; j++){
			if (processes[min_idx]->pid > processes[j]->pid){
				min_idx = j;
			}
		}

		struct ps* temp_proc = malloc(sizeof(struct ps));
		memcpy(temp_proc, processes[i], sizeof(struct ps));
		free(processes[i]); processes[i] = malloc(sizeof(struct ps));
		memcpy(processes[i], processes[min_idx], sizeof(struct ps));
		free(processes[min_idx]); processes[min_idx] = malloc(sizeof(struct ps));
		memcpy(processes[min_idx], temp_proc, sizeof(struct ps));
		free(temp_proc);
	}

	printf("%5s %5s %6s %s\n", "PID", "PPID", "STANJE", "IME");
	for(int i = 0; i < num_proc; i++){
		if(wanted_pid == -1 || (wanted_pid != -1 && (processes[i]->pid == wanted_pid || processes[i]->ppid == wanted_pid)))
			printf("%5d %5d %6s %s\n", processes[i]->pid, processes[i]->ppid, processes[i]->state, processes[i]->name);
		free(processes[i]);
	}

	free(processes);
	closedir(dir);
}

struct psext {
	int pid;
	int ppid;
	char state[1024];
	char name[1024];
	int threads;
	int open_files;
};

void get_process_threads_openfiles(char* path, int pid, int* threads, int* open_files){
	char* file_name = malloc(256 * sizeof(char));
	sprintf(file_name, "%s/%d/status", path, pid);

	// get number of threads
	FILE* file = fopen(file_name, "r");
	char* line = malloc(1024*sizeof(char));
	char* help_str = malloc(1024*sizeof(char));

	size_t len = 0;
	for(int i = 0; i < 23; i++)
		getline(&line, &len, file);

	fscanf(file, "%s %d", help_str, threads);	
	free(file_name); fclose(file);
	
	//get number of open files
	file_name = malloc(1024 * sizeof(char));
	sprintf(file_name, "%s/%d/fd", path, pid);
	
	DIR *dir = opendir(file_name);	
	struct dirent *content_dir = malloc(sizeof(struct dirent));

	int count_files = 0;
	while((content_dir = readdir(dir)) != NULL){
		count_files ++;
	}

	count_files -= 2;
	printf("%d", count_files);
	*open_files = count_files;

	free(file_name); 
	closedir(dir); free(content_dir);
}

void psext(char* path, int wanted_pid){

	DIR *dir = opendir(path);	
	struct dirent *content_dir = malloc(sizeof(struct dirent));

	int num_proc = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			num_proc ++;
		}
	}

	closedir(dir);
	dir = opendir(path);

	struct psext** processes = malloc(num_proc*sizeof(struct psext*));
	for(int i = 0; i < num_proc; i++){
		processes[i] = malloc(sizeof(struct psext));
	}

	int idx = 0;
	while((content_dir = readdir(dir)) != NULL){
		if(ispid(content_dir->d_name)){
			processes[idx]->pid = atoi(content_dir->d_name);
			get_process_ppid_state(path, processes[idx]->pid, &(processes[idx]->ppid), processes[idx]->state);
			strcpy(processes[idx]->name, get_process_name(path, processes[idx]->pid));
			get_process_threads_openfiles(path, processes[idx]->pid, &(processes[idx]->threads), &(processes[idx]->open_files));
			idx ++;
		}
	}
	
	for(int i = 0; i < num_proc; i++){
		int min_idx = i;
		for(int j = i + 1; j < num_proc; j++){
			if (processes[min_idx]->pid > processes[j]->pid){
				min_idx = j;
			}
		}

		struct ps* temp_proc = malloc(sizeof(struct psext));
		memcpy(temp_proc, processes[i], sizeof(struct psext));
		free(processes[i]); processes[i] = malloc(sizeof(struct psext));
		memcpy(processes[i], processes[min_idx], sizeof(struct psext));
		free(processes[min_idx]); processes[min_idx] = malloc(sizeof(struct psext));
		memcpy(processes[min_idx], temp_proc, sizeof(struct psext));
		free(temp_proc);
	}

	printf("%5s %5s %6s %6s %6s %s\n", "PID", "PPID", "STANJE", "#NITI", "#DAT.", "IME");
	for(int i = 0; i < num_proc; i++){
		if(wanted_pid == -1 || (wanted_pid != -1 && (processes[i]->pid == wanted_pid || processes[i]->ppid == wanted_pid)))
			printf("%5d %5d %6s %6d %6d %s\n", processes[i]->pid, processes[i]->ppid, processes[i]->state, processes[i]->threads, processes[i]->open_files, processes[i]->name);
		free(processes[i]);
	}

	free(processes);
	closedir(dir);
}

int main(int argc, char* argv[])
{
   
    if(strcmp(argv[1], "sys") == 0)   { 
        sys(argv[2]);
    } else if(strcmp(argv[1], "sysext") == 0){
    	sysext(argv[2]);
    } else if(strcmp(argv[1], "me") == 0){
    	me();
    } else if(strcmp(argv[1], "pids") == 0){
    	pids(argv[2]);
    } else if(strcmp(argv[1], "names") == 0){
    	names(argv[2]);
    } else if(strcmp(argv[1], "ps") == 0){
    	if(argc == 3)
	    	ps(argv[2], -1);
	   	else if (argc == 4){
	   		ps(argv[2], atoi(argv[3]));
	   	} 
    } else if(strcmp(argv[1], "psext") == 0){
    	if(argc == 3)
	    	psext(argv[2], -1);
	   	else if (argc == 4){
	   		psext(argv[2], atoi(argv[3]));
	   	} 
    }

 	exit(0);
}
