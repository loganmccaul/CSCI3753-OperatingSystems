/*Logan McCaul
 * OS PA2 CSCI3753
 * University of Colorado */
 
#include "multi-lookup.h"

#define MINARGS 3
#define USAGE "<inputFilePath> <outputFilePath>"
#define SBUFSIZE 1025
#define INPUTFS "%1024s"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cp = PTHREAD_COND_INITIALIZER;
pthread_cond_t cc = PTHREAD_COND_INITIALIZER;
queue q;
int pdone = 0;
FILE* outputfp = NULL;


void* producer(void* inputFile){
	char hostname[MAX_NAME_LENGTH];
	char* hostnameGive;
	FILE* inputfp = fopen(inputFile, "r");
	
	while((fscanf(inputfp, INPUTFS, hostname)) > 0){
			//printf("prod 1\n");
			pthread_mutex_lock(&m);
			while(queue_is_full(&q) == 1){
				//printf("prod 2\n");
				pthread_cond_wait(&cp, &m);
			}
			hostnameGive = malloc((MAX_NAME_LENGTH)*sizeof(char));
			strcpy(hostnameGive, hostname);
			queue_push(&q, hostnameGive);
			pthread_cond_broadcast(&cc);
			pthread_mutex_unlock(&m);
			printf("Producer: put %s\n",hostname);
	}
	fclose(inputfp);

	//pthread_exit(NULL);
	return NULL;
}

void* consumer(){
	//char hostname[MAX_NAME_LENGTH];
	char* hostname;
	char firstipstr[MAX_IP_LENGTH];
		while(pdone==0){
			//printf("cons 1\n");
			pthread_mutex_lock(&m);
			while(queue_is_empty(&q) == 1){
				//printf("cons 2\n");
				pthread_cond_wait(&cc, &m);
				}
			if((hostname = (char*)queue_pop(&q)) != NULL){
				//printf("hostname is not there");
			//strcpy(hostname, hostnameGet);
			// Lookup hostname and get IP string
			if(dnslookup(hostname, firstipstr, sizeof(firstipstr))
		       == UTIL_FAILURE){
				fprintf(stderr, "dnslookup error: %s\n", hostname);
				strncpy(firstipstr, "", sizeof(firstipstr));
		    }
		    // Write to Output File
		    fprintf(outputfp, "%s,%s\n", hostname, firstipstr);
			printf("Consumer: got %s\n",hostname);
			pthread_cond_broadcast(&cp);
			pthread_mutex_unlock(&m);
			//if(hostname){
				free(hostname);
				hostname = NULL;
		}
		}
		printf("Queue is empty\n");
		//pthread_exit(NULL);
		return NULL;
}

int main(int argc, char* argv[]){
	
	//Declarations
	int numThreads = argc;
	//FILE* inputfp = NULL;
	pthread_t pThreads[MAX_RESOLVER_THREADS];
	pthread_t cThreads[MAX_RESOLVER_THREADS];
	char error[MAX_NAME_LENGTH];
	
	printf("%d\n", numThreads);
	printf("FINISHED SETUP\n\n");
	
	//Checks to see if minimum is reached
	if(argc < MINARGS){
		fprintf(stderr, "Not enough arguments: %d\n", (argc - 1));
		fprintf(stderr, "Usage:\n %s %s\n", argv[0], USAGE);
	return EXIT_FAILURE;
	}
	//Initializers
	queue_init(&q, 50);
	pthread_mutex_init(&m, NULL);
	pthread_cond_init(&cp, NULL);
	pthread_cond_init(&cc, NULL);

	printf("Everything Initialized\n\n");
	
	//Open Output
	outputfp = fopen(argv[(argc-1)], "w");
	
	//Checks if opened correctly
	if(!outputfp){
		sprintf(error, "Error opening Output File\n\n");
		perror(error);
		return EXIT_FAILURE;
	}
	else{
		printf("Output File Opened\n\n");
	}
	
	//Create Threads
	int i;
	for(i=1; i < numThreads; i++){
		//inputfp = fopen(argv[i+2], "r");
		/*if(!inputfp){//Checks if opened correctly
			sprintf(error, "Error Opening Input File: %s", argv[i+1]);
			perror(error);
			queue_cleanup(&q);
			return EXIT_FAILURE;
		}
		else{
			printf("Input file Opened\n");
		}*/
		printf("Created Producer %d\n", i);
		printf("Created Consumer %d\n", i);
		
		//Producer threads
		int rp = pthread_create(&(pThreads[i-1]), NULL, producer, argv[i]);

		if(rp){
			printf("Error creating producer thread\n");
			return EXIT_FAILURE;
		}
		
		//Consumer threads
		int rc = pthread_create(&(cThreads[i-1]), NULL, consumer, NULL);
		if(rc){
			printf("Error creating consumer thread\n");
			return EXIT_FAILURE;
		}
		printf("Thread Created\n\n");
	}
		
	//Threads wait for eachother to finish
	for(i=0; i<numThreads-1; i++){
		pthread_join(pThreads[i],NULL);
	}
	pdone = 1;
	for(i=0; i<numThreads-1; i++){
		pthread_join(cThreads[i],NULL);
	}
	printf("Threads joined\n\n");
	
	//close output file
	fclose(outputfp);
	queue_cleanup(&q);
	pthread_mutex_destroy(&m);
	pthread_cond_destroy(&cp);
	pthread_cond_destroy(&cc);
	
	printf("Finished Succesfully\n");
	return EXIT_SUCCESS;
	
}
