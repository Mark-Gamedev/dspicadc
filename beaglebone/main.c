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
#include <signal.h>

#include "spi.h"
#include "plotGraph.h"
#include "tcpServer.h"
#include "calibration.h"
#include "processData.h"

#define LOCATIONS 100
#define SAMPLENUM 1 

extern int xcorr(int*, int*, int, double*);
extern int maxIndex(double *, int , double *);

int exitProgram = 0;
void interruptHandler(int signum){
	exitProgram = 1;
}

void saveWaveData(FILE * file,int *buf[LOCATIONS][SAMPLENUM]){
	int i=0,j=0,k=0;
	fprintf(file,"%d\n",LOCATIONS);
	fprintf(file,"%d\n",SAMPLENUM);
	fprintf(file,"%d\n",512);

	for(i=0;i<LOCATIONS;i++){
		for(j=0;j<SAMPLENUM;j++){
			for(k=0;k<512;k++){
				fprintf(file,"%d\n",buf[i][j][k]);
			}
			fprintf(file,"%d\n",-1);
		}
		fprintf(file,"%d\n",-2);
	}
}

void loadWaveData(FILE * file,int *buf[LOCATIONS][SAMPLENUM]){
	int i=0,j=0,k=0;	
	int temp;
	fscanf(file,"%d",&temp);
	if(temp!=LOCATIONS){
		printf("LOCATIONS incorrect\n");
		exit(1);
	}
	fscanf(file,"%d",&temp);
	if(temp!=SAMPLENUM){
		printf("SAMPLENUM incorrect\n");
		exit(1);
	}
	fscanf(file,"%d",&temp);
	if(temp!=512){
		printf("size incorrect\n");
		exit(1);
	}
	for(i=0;i<LOCATIONS;i++){
		for(j=0;j<SAMPLENUM;j++){
			if((buf[i][j]=(int*)calloc(512,sizeof(int)))==NULL){
				printf("calloc failed\n");
				exit(1);
			}
			for(k=0;k<512;k++){
				
				if((fscanf(file,"%d",&buf[i][j][k])==EOF)){
					printf("end of file prematurely\n");
					exit(1);
				}
			}
			fscanf(file,"%d",&temp);
			if(temp!=-1){
				printf("expected -1, file corrupted\n");
				exit(1);
			}
		}
		fscanf(file,"%d",&temp);
		if(temp!=-2){
			printf("expected -2, file corrupted\n");
			exit(1);
		}
	}
}

int file_exists(char * filename){
	FILE * file;
	if((file = fopen(filename,"r"))){
		fclose(file);
		return 1;
	} 
	return 0;
}

void reverseArray(int * array,int len){
	int i=0;
	int temp;
	len=len-1;
	for(i=0;i<len/2;i++){
		temp=array[i];
		array[i]=array[len-i];
		array[len-i]=temp;
	}

}

void defineLocations(int * buf[LOCATIONS][SAMPLENUM]){
	int i=0,j=0, sz;
	char msg[128];

	sprintf(msg, "%d", -1);
	sendToServer((char*)msg, sizeof(msg));
	for(i=0;i<LOCATIONS;i++){
		for(j=0;j<SAMPLENUM;j++){
			saveBufferFromSpi(&buf[i][j],&sz);
			sleep(1);
			printf("got %d, %d\n",i,j);
			sprintf(msg, "%d", i);
			sendToServer((char*)msg, sizeof(msg));
		}
	}
}

void runOnce(int *waves[LOCATIONS][SAMPLENUM]){
	int *buf, *ch0;
	int sz;
	char msg[32];
	double xcorrOut[512*2];
	double locationCorr[LOCATIONS];
	int locationCorrInt[LOCATIONS];
	double xCorr;
	int locationHit;

	saveBufferFromSpi(&buf, &sz);
	ch0 = buf;

	int i=0;
	for(i=0;i<LOCATIONS;i++){
		locationCorr[i]=0;
		int j=0;
		for(j=0;j<SAMPLENUM;j++){
			xcorr(waves[i][j],ch0,512,xcorrOut);
			maxIndex(xcorrOut,512,&xCorr);
			locationCorr[i]+=xCorr;
		}
		locationCorr[i]=locationCorr[i]/SAMPLENUM;
	}

	locationHit=maxIndex(locationCorr,LOCATIONS,NULL);
	printf("%d\n,corr=%lf\n",locationHit,locationCorr[locationHit]);

	//send entire locationCorr array
	sendToServer((char*)locationCorr, sizeof(locationCorr));

	bzero(msg, sizeof(msg));
	//sprintf(msg, "%d", locationHit);
	//sendToServer((char*)msg, sizeof(msg));

	for(i=0;i<LOCATIONS;i++)
		locationCorrInt[i]=(int)(locationCorr[i]*100);

	saveBufferToFile(locationCorrInt, LOCATIONS, "/tmp/locCorr");
	
	free(buf);
}

int main(int argc, char *argv[]){
	int *buf[LOCATIONS][SAMPLENUM];
	FILE * file;

	signal(SIGINT, interruptHandler);

	printf("connecting to SPI...");
	fflush(stdout);
	spiInit();
	printf("done\n");

	printf("waiting for TCP connection...");
	fflush(stdout);
	startServer();
	printf("done\n");

	if(argc!=2){
		printf("please enter a filename\n");
		exit(1);
	} else {
		if(file_exists(argv[1])){
			printf("file exists\n");
			file=fopen(argv[1],"r");
			printf("file opened\n");
			loadWaveData(file,buf);
			printf("loaded wave data\n");
		} else {
			printf("file does not exist\n");
			file=fopen(argv[1],"w");
			defineLocations(buf);
			saveWaveData(file,buf);
			printf("saved wave data\n");
		}
		fclose(file);
	}

	while(!exitProgram){
		runOnce(buf);
		usleep(100000);
	}

	cleanupServer();
	spiCleanup();
	return 0;
}

