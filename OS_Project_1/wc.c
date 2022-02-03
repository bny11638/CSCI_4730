#include "wc.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void child_process(int i, int pipe_write, int pipe_read, char **argv, int partitionSize,
					  int *childPids, int nChildProc, int fsize) {
			childPids[i] = fork();
			if (childPids[i] < 0) { 
				fprintf(stderr, "Fork Failed");
				// return 1;
			} 
			if (childPids[i] > 0) {}
			else {
				count_t tmpCount;	
				// Open n different file pointers in read-only mode
				FILE *fpChild = fopen(argv[1], "r");
				if(fpChild == NULL) {
					printf("File open error: %s\n", argv[1]);
					printf("usage: wc <filname>\n");
					// return 0;
				}
				int bytesToRead = partitionSize;
				if ( i == nChildProc - 1) {
					bytesToRead = fsize - (i*partitionSize);
				}
				tmpCount = word_count(fpChild, i*partitionSize , bytesToRead);
				// close(pipe_read);
				// writing the memory address of the struct into the pipe
				write(pipe_write, &tmpCount, sizeof(count_t));
				exit(0);
			}  
}


int main(int argc, char **argv)
{
		long fsize;
		FILE *fp;
		count_t count;
		struct timespec begin, end;
		int nChildProc = 0;
		count_t bufferStruct;
		int iteration = 0;	
		int success =0;	
		/* 1st arg: filename */
		if(argc < 2) {
				printf("usage: wc <filname> [# processes] [crash rate]\n");
				return 0;
		}
		
		/* 2nd (optional) arg: number of child processes */
		if (argc > 2) {
				nChildProc = atoi(argv[2]);
				if(nChildProc < 1) nChildProc = 1;
				if(nChildProc > 10) nChildProc = 10;
		}

		/* 3rd (optional) arg: crash rate between 0% and 100%. Each child process has that much chance to crash*/
		if(argc > 3) {
				crashRate = atoi(argv[3]);
				if(crashRate < 0) crashRate = 0;
				if(crashRate > 50) crashRate = 50;
				printf("crashRate RATE: %d\n", crashRate);
		}
		
		printf("# of Child Processes: %d\n", nChildProc);
		printf("crashRate RATE: %d\n", crashRate);

		count.linecount = 0;
		count.wordcount = 0;
		count.charcount = 0;

		// start to measure time
		clock_gettime(CLOCK_REALTIME, &begin);

		
		// get a file size
		fp = fopen(argv[1], "r");
		fseek(fp, 0L, SEEK_END);
		fsize = ftell(fp);
		
		pid_t childPids[nChildProc];
		
		/* word_count() has 3 arguments.
			* 1st: file descriptor
			* 2nd: starting offset
			* 3rd: number of bytes to count from the offset
			*/
		int partitionSize = fsize / nChildProc;
		//int pipes[nChildProc][2];
		int p[2];
		if (pipe(p) == -1) {
			fprintf(stderr, "Pipe Creation Failed");
			return 1;
		}
		int pids[nChildProc];
		for (int i = 0; i < nChildProc; i++) {
			/**
			if (pipe(pipes[i]) == -1 ) {
				fprintf(stderr, "Pipe Creation Failed");
				return 1;
			}
			*/
			child_process(i, p[1], p[0], argv, 
						partitionSize, childPids, nChildProc, fsize);
		}
		int num_fail = 0;
		while (success < nChildProc) {
    			int signal = 0;
			int n = 0;
			int childPid = waitpid(-1, &signal, 0);
			// this is necessary to find which process it is 1-n?
			while (n < nChildProc && childPid != childPids[n]) n++;
			if (signal == 0) {
				success++;
				//close(pipes[n][1]);
				// close(p[1]);
				// reading the sturct from the memory address of the pipe
				// read(pipes[n][0],&bufferStruct,sizeof(count_t));
				read(p[0],&bufferStruct,sizeof(count_t));
				count.linecount = count.linecount + bufferStruct.linecount;
				count.wordcount = count.wordcount + bufferStruct.wordcount;
				count.charcount = count.charcount + bufferStruct.charcount;
			} else {
				num_fail = num_fail + 1;
				child_process(n, p[1], p[0], argv, 
						partitionSize, childPids, nChildProc, fsize);
			}
		}
		fclose(fp);
		close(p[0]);
		close(p[1]);
		clock_gettime(CLOCK_REALTIME, &end);
		long seconds = end.tv_sec - begin.tv_sec;
		long nanoseconds = end.tv_nsec - begin.tv_nsec;
		double elapsed = seconds + nanoseconds*1e-9;
		printf("Total Failures: %d \n", num_fail); 
		printf("\n========= %s =========\n", argv[1]);
		printf("Total Lines : %d \n", count.linecount);
		printf("Total Words : %d \n", count.wordcount);
		printf("Total Characters : %d \n", count.charcount);
		printf("======== Took %.3f seconds ========\n", elapsed);
		
		
		return(0);
}

