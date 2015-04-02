#include "multi-lookup.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t c = PTHREAD_COND_INITIALIZER;
queue q;
int count;

void* inputThread(void* f){
	/* Lookup hostname and get IP string */
	inThread* in = (inThread*) f;
    char* hostname = (char*)malloc((MAX_NAME_LENGTH)*sizeof(char));
    //char strings[MAX_NAME_LENGTH];  
    int x= 0;
  
    while(fscanf(in->inputfp, INPUTFS, hostname) > 0){
	    //strcpy(strings,hostname);
		while(queue_is_full(&q)){
			int rnd = 1 + (int)( 100.0 * rand() / ( RAND_MAX + 1.0 ) );
			usleep(rnd);
		}
		printf("Awake\n\n");
		pthread_mutex_lock(&m);
		printf("\n%s\n", hostname);
		queue_push(&q, hostname);
		x++;
		pthread_mutex_unlock(&m);
		
		printf("producer: put %d\n", x);
	}
	fclose(in->inputfp);
	free(hostname);
	hostname=NULL;
	pthread_mutex_lock(&m);
	
	count-=1;
	printf("Quit producer OK \n");
	pthread_mutex_unlock(&m);
	printf("MUTEX unlocked\n\n");
	//fclose(in->inputfp);
	pthread_exit(NULL);
	return NULL;
}

void* outputThread(void* f){
	outThread* out = (outThread*) f;
    char* hostname;
   // char strings[MAX_NAME_LENGTH];
    char firstipstr[MAX_IP_LENGTH];
    
    int y=0;
	while(1){
		if(!queue_is_empty(&q)){	
			printf("Testing queue in Consumer\n");
		pthread_mutex_lock(&m);
		if(!queue_is_empty(&q)){  
			if ((hostname = (char*)queue_pop(&q)) != NULL){
			printf("\n\n     %s\n", hostname);
		}
		}
		printf("Copy string down in consumer\n");
			if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
				== UTIL_FAILURE){
				fprintf(stderr, "dnslookup error: %s\n", hostname);
				strncpy(firstipstr, "", sizeof(firstipstr));
				printf("Succesfully written string\n");
			}
		y++;
		
		/*strcpy(strings,hostname);
        strcat(strings,","); 
		strcat(strings,firstipstr);
		fprintf(out->outputfp, "%s\n", strings);*/
		printf("   \n\n%s, %s\n\n", hostname, firstipstr);
		fprintf(out->outputfp, "%s,%s\n", hostname, firstipstr);
		
		free(hostname);
		hostname= NULL;

		printf("consumer: got %d\n", y);
		printf("write in file good\n");
		pthread_mutex_unlock(&m);
		}
		//printf("\n\n Queue Values: %d\n", queue_is_empty(&q));
		//printf(" Count: %d\n\n", count);
		if(queue_is_empty(&q)){
			printf("Quit from consumer\n");
			printf("consumer consumed %d items\n\n", y);
			return NULL;
		}

	}
	pthread_exit(NULL);
	return NULL;
}

int main(int argc, char* argv[]){
	
	/*Local Vars */
    int num_threads = argc-2;
    pthread_t ithreads[MAX_RESOLVER_THREADS];
    pthread_t othreads[MAX_RESOLVER_THREADS];
    inThread in[num_threads];
    outThread out[num_threads];
    FILE* inputfp = NULL;
    FILE* outputfp = NULL;   
    char errorstr[MAX_NAME_LENGTH];
   // pthread_mutex_t m;
    
    printf("%d",num_threads);
    count=num_threads;
    
    printf("Setup OK \n");
    
    /* Check Arguments */
    if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
	return EXIT_FAILURE;
    }
    
     /* initialize queue */
     int q_size = 100;
     if(queue_init(&q, q_size) == QUEUE_FAILURE){
		fprintf(stderr, "error: queue_init failed!\n");
     }
     pthread_mutex_init(&m, NULL);     
     pthread_mutex_unlock(&m);
     
     printf("Queue OK \n");

    /* Open Output File */
    outputfp = fopen(argv[(argc-1)], "w");
    if(!outputfp){
		perror("Error Opening Output File");
		return EXIT_FAILURE;
	}

	printf("Output file OK \n");
	 /*create threads*/
	long i;
    for(i=0;i<num_threads;i++){
	
        inputfp = fopen(argv[i+1], "r");
        if(!inputfp){
			sprintf(errorstr, "Error Opening Input File: %s", argv[i+1]);
			perror(errorstr);
            queue_cleanup(&q);
	    return EXIT_FAILURE;
        }
        printf("Creates Queue\n");
        out[i].outputfp = outputfp;
        in[i].inputfp = inputfp;

        /*out[i].q = &q;
  
        in[i].q = &q;

		
        
        out[i].m = &m;
        in[i].m = &m;*/

        printf("In main: creating consumer thread %ld\n", i);
        printf("In main: creating producer thread %ld\n", i);
        
        int rc = pthread_create(&(ithreads[i]), NULL, inputThread, &(in[i]));
		if (rc){
			printf("ERROR; return code from pthread_create() is %d\n", rc);
			exit(EXIT_FAILURE);
		}
        
		int rp = pthread_create(&(othreads[i]), NULL, outputThread, &(out[i]));
		if (rp){
			printf("ERROR; return code from pthread_create() is %d\n", rp);
			exit(EXIT_FAILURE);
		}
		printf("Create thread OK \n");
	}
	
	/* Wait for All Threads to Finish */
	for(i=0;i<num_threads;i++){
        pthread_join(othreads[i],NULL);
    }
    for(i=0;i<num_threads;i++){
		pthread_join(ithreads[i],NULL);
    }
    
	printf("Wait for thread OK \n");
	
    fclose(outputfp);
   
    printf("All of the threads were completed!\n");
    queue_cleanup(&q);

    return EXIT_SUCCESS;
}
