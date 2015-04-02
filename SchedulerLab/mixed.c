#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <math.h>

/* Local Defines */
#define DEFAULT_ITERATIONS 1000000
#define RADIUS (RAND_MAX / 2)
#define MAXFILENAMELENGTH 80
#define DEFAULT_INPUTFILENAME "rwinput"
#define DEFAULT_OUTPUTFILENAMEBASE "rwoutput"
#define DEFAULT_BLOCKSIZE 1024
#define DEFAULT_TRANSFERSIZE 1024*100

/* Local Functions */
inline double dist(double x0, double y0, double x1, double y1){
    return sqrt(pow((x1-x0),2) + pow((y1-y0),2));
}

inline double zeroDist(double x, double y){
    return dist(0, 0, x, y);
}


int main(int argc, char* argv[]){
	long i;
    long iterations;
    double x, y;
    double inCircle = 0.0;
    double inSquare = 0.0;
    double pCircle = 0.0;
    double piCalc = 0.0;
    
    int rv;
    int outputFD;
    char outputFilename[MAXFILENAMELENGTH];
    char outputFilenameBase[MAXFILENAMELENGTH];

    ssize_t transfersize = 0;
    ssize_t blocksize = 0; 
    char* transferBuffer = NULL;
    ssize_t buffersize;

    ssize_t bytesWritten = 0;
    ssize_t totalBytesWritten = 0;
	int totalWrites = 0;
    
       /* Process program arguments to select iterations */
    /* Set default iterations if not supplied */
    if(argc < 2){
	iterations = DEFAULT_ITERATIONS;
    }
    /* Set iterations if supplied */
    else{
	iterations = atol(argv[1]);
	if(iterations < 1){
	    fprintf(stderr, "Bad iterations value\n");
	    exit(EXIT_FAILURE);
	}
    }
    
     if(argc < 3){
	transfersize = DEFAULT_TRANSFERSIZE;
    }
    else{
	transfersize = atol(argv[2]);
	if(transfersize < 1){
	    fprintf(stderr, "Bad transfersize value\n");
	    exit(EXIT_FAILURE);
	}
    }
    /* Set supplied block size or default if not supplied */
    if(argc < 4){
	blocksize = DEFAULT_BLOCKSIZE;
    }
    else{
	blocksize = atol(argv[3]);
	if(blocksize < 1){
	    fprintf(stderr, "Bad blocksize value\n");
	    exit(EXIT_FAILURE);
	}
    }
    
    if(argc < 5){
	if(strnlen(DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
	    fprintf(stderr, "Default output filename base too long\n");
	    exit(EXIT_FAILURE);
	}
	strncpy(outputFilenameBase, DEFAULT_OUTPUTFILENAMEBASE, MAXFILENAMELENGTH);
    }
    else{
	if(strnlen(argv[4], MAXFILENAMELENGTH) >= MAXFILENAMELENGTH){
	    fprintf(stderr, "Output filename base is too long\n");
	    exit(EXIT_FAILURE);
	}
	strncpy(outputFilenameBase, argv[4], MAXFILENAMELENGTH);
    }
    
    
    /* Confirm blocksize is multiple of and less than transfersize*/
    if(blocksize > transfersize){
	fprintf(stderr, "blocksize can not exceed transfersize\n");
	exit(EXIT_FAILURE);
    }
    if(transfersize % blocksize){
	fprintf(stderr, "blocksize must be multiple of transfersize\n");
	exit(EXIT_FAILURE);
    }
    
    buffersize = blocksize;
    if(!(transferBuffer = malloc(buffersize*sizeof(*transferBuffer)))){
	perror("Failed to allocate transfer buffer");
	exit(EXIT_FAILURE);
    }
    
	/* Calculate pi using statistical methode across all iterations*/
    for(i=0; i<iterations; i++){
	x = (random() % (RADIUS * 2)) - RADIUS;
	y = (random() % (RADIUS * 2)) - RADIUS;
	if(zeroDist(x,y) < RADIUS){
	    inCircle++;
	}
	inSquare++;
    }

    /* Finish calculation */
    pCircle = inCircle/inSquare;
    piCalc = pCircle * 4.0;
    
    rv = snprintf(outputFilename, MAXFILENAMELENGTH, "%s-%d",
		  outputFilenameBase, getpid());    
    if(rv > MAXFILENAMELENGTH){
	fprintf(stderr, "Output filenmae length exceeds limit of %d characters.\n",
		MAXFILENAMELENGTH);
	exit(EXIT_FAILURE);
    }
    else if(rv < 0){
	perror("Failed to generate output filename");
	exit(EXIT_FAILURE);
    }
    if((outputFD =
	open(outputFilename,
	     O_WRONLY | O_CREAT | O_TRUNC | O_SYNC,
	     S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH)) < 0){
	perror("Failed to open output file");
	exit(EXIT_FAILURE);
    }

	//fprintf(stdout, "Reading %s and writing to %s\n",
	    //piCalc, outputFilename);
	    
	 bytesWritten = write(outputFD, transferBuffer, piCalc);
	  if(bytesWritten < 0){
		perror("Error writing output file");
		exit(EXIT_FAILURE);
	  }
	  else{
		totalBytesWritten += bytesWritten;
		totalWrites++;
	    }
	   
	   fprintf(stdout, "Written: %zd bytes in %d writes\n",
	    totalBytesWritten, totalWrites);
	   fprintf(stdout, "Processed %zd bytes in blocks of %zd bytes\n",
	    transfersize, blocksize);
	  
	   free(transferBuffer);

    /* Close Output File Descriptor */
    if(close(outputFD)){
	perror("Failed to close output file");
	exit(EXIT_FAILURE);
    }
	    
	return EXIT_SUCCESS;
}
