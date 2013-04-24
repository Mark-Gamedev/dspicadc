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


int delayData[9][2] = {
	{8, 6}, {2, 5}, {-4, 3},
	{4, 1}, {-2, -2}, {-8, -1},
	{2, -1}, {-4, -8}, {-8, -6}};

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


#define MARGIN 2
int calculateCoord(int d0, int d1){
	//TODO: calculate margin and pick the lowest margin
	int i;
	for(i=0;i<9;i++){
		if(d0 >= delayData[i][0]-MARGIN && d0 <= delayData[i][0]+MARGIN && 
				d1 >= delayData[i][1]-MARGIN && d1 <= delayData[i][1]+MARGIN){
			return i;
		}
	}
	return -1;
}

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
	int msg[3];

	printf("ready...");
	fflush(stdout);

	saveBufferFromSpi(&buf, &sz);
	printf("processing...\n");
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];

	performThreshold(ch0, ch1, ch2, chsz, &d0, &d1, &d2);

	msg[0] = d0;
	msg[1] = d1;
	msg[2] = d2;
	sendToServer((char*)msg, sizeof(msg));

	/*
	int i;
	for(i=0;i<20;i++){
		fputs("\033[A\033[2K", stdout);
	}
	*/
	printf("%d %d %d\n", d0, d1, d2);
	//printCoord(calculateCoord(d0-d2, d1-d2));

	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");
	plotWithGnuplot("sss", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2");

	free(buf);
}

int main(int argc, char *argv[]){
	printf("connecting to SPI...");
	fflush(stdout);
	spiInit();
	printf("done\n");

	calibrate(3, 5);
	printCalibrationData();
	exit(0);

	printf("waiting for TCP connection...");
	fflush(stdout);
	//startServer();
	printf("done\n");

	while(1){
		runOnce();
	}

	spiCleanup();
	return 0;
}

