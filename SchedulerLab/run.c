#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char* argv[]){
	int policy;
	int i = 0;
	long iterations;
	int type = 0;
	pid_t pid;
	struct sched_param param;
	/*type 1 = mixed
	 *type 2 = cpu
	 *type 3 = I/O
	 */
	
	/* Process program arguments to select iterations and policy */
    
    /* Set default iterations if not supplied */
    if(argc < 2){
	iterations = 50;
    }
    /* Set default policy if not supplied */
    if(argc < 3){
	policy = SCHED_OTHER;
    }
    if(argc < 4){
	type = 1;	
	}
    
    /* Set iterations if supplied */
    if(argc > 1){
	iterations = atol(argv[1]);
	if(iterations < 1){
	    fprintf(stderr, "Bad iterations value\n");
	    exit(EXIT_FAILURE);
	}
    }
    /* Set policy if supplied */
    if(argc > 2){
	if(!strcmp(argv[2], "SCHED_OTHER")){
	    policy = SCHED_OTHER;
	}
	else if(!strcmp(argv[2], "SCHED_FIFO")){
	    policy = SCHED_FIFO;
	}
	else if(!strcmp(argv[2], "SCHED_RR")){
	    policy = SCHED_RR;
	}
	else{
	    fprintf(stderr, "Unhandeled scheduling policy\n");
	    exit(EXIT_FAILURE);
	}
    }
    
    if(argc > 3){
	if(!strcmp(argv[3], "mixed")){
	    type = 1;
	}
	else if(!strcmp(argv[3], "cpu")){
	    type = 2;
	}
	else if(!strcmp(argv[3], "io")){
	    type = 3;
	}
	else{
	    fprintf(stderr, "Unhandeled scheduling policy\n");
	    exit(EXIT_FAILURE);
	}
    }
    
    /* Set process to max prioty for given scheduler */
    param.sched_priority = sched_get_priority_max(policy);
    
    /* Set new scheduler policy */
    fprintf(stdout, "Current Scheduling Policy: %d\n", sched_getscheduler(0));
    fprintf(stdout, "Setting Scheduling Policy to: %d\n", policy);
   
    if(sched_setscheduler(0, policy, &param)){
		perror("Error setting scheduler policy");
		exit(EXIT_FAILURE);
    }
    
    fprintf(stdout, "New Scheduling Policy: %d\n", sched_getscheduler(0));
	
	for(i =0; i < iterations; i++){
		pid=fork();
		if(pid==0){
			switch(type){
				case 1:
					system("./mixed");
					break;
				case 2:
					system("./pi");
					break;
				case 3:
					system("./rw");
					break;
			}
		exit(EXIT_SUCCESS);
		}
		else if(pid<0){
			perror("Error forking child");
			exit(EXIT_FAILURE);
		}
	}
	
	if(pid!=0)
		while (waitpid(-1, NULL, 0) > 0);
	return 0;
}
