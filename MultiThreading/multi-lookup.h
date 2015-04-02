#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

#include "util.h"
#include "queue.h"

#ifndef MULTI_LOOKUP_H
#define MULTI_LOOKUP_H

#define MAX_INPUT_FILES 10
#define MAX_RESOLVER_THREADS 10
#define MIN_RESOLVER_THREADS 2
#define MAX_NAME_LENGTH 1025
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

/*typedef struct {
	FILE* inputfp;
}producerArgs;

typedef struct {
	FILE* outputfp;
}consumerArgs;*/

void* producer(void* inputFile);
void* consumer();

#endif
