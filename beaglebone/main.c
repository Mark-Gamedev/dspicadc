#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <linux/types.h>

#include "spi.h"
#include "plotGraph.h"
#include "tcpServer.h"
#include "calibration.h"
#include "processData.h"


int **delayData;

/*
void performFirstPeak(){
	int *buf, *ch0, *ch1, *ch2, index0, index1, index2;
	int sz, chsz;
	printf("waiting for knock...\n");
	saveBufferFromSpi(&buf, &sz);
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];
	index0 = firstPeak(ch0, chsz, THRESHOLD);
	index1 = firstPeak(ch1, chsz, THRESHOLD);
	index2 = firstPeak(ch2, chsz, THRESHOLD);
	printf("i0: %d, i1: %d, i2: %d\n", index0, index1, index2);

	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");
}

void performXCorr(int *ch0, int *ch1, int *ch2, int chsz, int *d01, int *d02, int *d12){
	double *corr, maxVal;

	int sz = chsz*2;
	corr = calloc(sz, sizeof(double));

	xcorr(ch0, ch1, chsz, corr);
	*d01 = maxIndex(corr, sz, &maxVal);
	*d01 -= chsz;

	xcorr(ch0, ch2, chsz, corr);
	*d02 = maxIndex(corr, sz, &maxVal);
	*d02 -= chsz;

	xcorr(ch1, ch2, chsz, corr);
	*d12 = maxIndex(corr, sz, &maxVal);
	*d12 -= chsz;
}
*/

void printCoord(int x){
	int i;
	for(i=0;i<9;i++){
		if(i==x){
			printf("    X    ");
		}else{
			printf("    -    ");
		}
		if(i==2||i==5||i==8){
			printf("\n\n\n");
		}
	}
}

void runOnce(){
	int *buf, *ch0, *ch1, *ch2;
	int d0, d1, d2;
	int sz, chsz;

	printf("ready...");
	fflush(stdout);

	saveBufferFromSpi(&buf, &sz);
	printf("processing...\n");
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];

	performThreshold(ch0, ch1, ch2, chsz, &d0, &d1, &d2);


	/*
	int i;
	for(i=0;i<20;i++){
		fputs("\033[A\033[2K", stdout);
	}
	*/
	//printf("%d %d %d\n", d0, d1, d2);
	int result = calculateLocation(d0, d1, d2);
	printf("r:%d\n", result);

	free(buf);
}

int main(int argc, char *argv[]){
	printf("connecting to SPI...");
	fflush(stdout);
	spiInit();
	printf("done\n");

	calibrate(9, 10);

	printf("waiting for TCP connection...");
	fflush(stdout);
	//startServer();
	printf("done\n");

	while(1){
		runOnce();
		sleep(1);
	}

	spiCleanup();
	return 0;
}

