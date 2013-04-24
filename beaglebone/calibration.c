#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "spi.h"
#include "processData.h"
#include "plotGraph.h"

#define CALIBRATION_DATA_PATH "calibration-data"
#define CALIBRATION_MARGIN 5

int *calibrationData, numOfPts, intPerPt;

int getBestSample(int *array, int x, int y, int margin){
	int i;
	int *err;
	int max;
	int index;
	int marginSq = margin*margin;

	err = (int*)calloc(x, sizeof(int));
	for(i=0;i<x;i++){
		int j;
		err[i] = 0;
		for(j=0;j<x;j++){
			int k;
			int marginSum = 0;
			for(k=0;k<y;k++){
				int s = array[j*y+k] - array[i*y+k];
				marginSum += s*s;
			}
			if(marginSum < marginSq){
				err[i] += 1;
			}
		}
	}
	max = 0;
	for(i=0;i<x;i++){
		if(err[i] > max){
			max = err[i];
			index = i;
		}
	}
	free(err);
	printf("m:%d\n", max);
	return index;
}


void printCalibrationData(){
	int i;
	for(i=0;i<numOfPts;i++){
		int j = 0;
		for(;j<intPerPt;j++){
			printf("%d ",calibrationData[i*intPerPt+j]);
		}
		printf("\n");
	}
}

int readArray(char *path, int *outx, int *outy, int **array){
	FILE *fd;
	int x, y;
	int i, j;
	int tmp;
	int *data;

	fd = fopen(path, "r");

	if(fscanf(fd, "%d", &x)!=1){
		return -1;
	}
	if(fscanf(fd, "%d", &y)!=1){
		return -1;
	}

	data = calloc(x*y, sizeof(int*));

	for(i=0; i<x; i++){
		for(j=0; j<y; j++){
			if(fscanf(fd, "%d", &tmp)!=1){
				free(data);
				return -1;
			}
			data[i*y+j] = tmp;
		}
	}

	*outx = x;
	*outy = y;
	*array = data;
	fclose(fd);
	return 0;
}

int writeArrayToFile(char *path, int x, int y, int* array){
	int i, j;
	FILE *fd;

	fd = fopen(path, "w");
	fprintf(fd, "%d %d\n", x, y);
	for(i=0;i<x;i++){
		for(j=0;j<y;j++){
			fprintf(fd, "%d ", array[i*y+j]);
		}
		fprintf(fd, "\n");
	}
	fclose(fd);
	return 0;
}

void calibrate(int numOfPoints, int numOfSamples){
	int i;

	int *buf, *ch0, *ch1, *ch2;
	int sz, chsz;

	calibrationData = calloc(numOfPoints*3, sizeof(int));
	numOfPts = numOfPoints;
	intPerPt = 3;


	for(i=0;i<numOfPoints;i++){
		int j;
		int *samples = calloc(numOfSamples*3, sizeof(int));
		for(j=0;j<numOfSamples;j++){

			printf("waiting for sample %d/%d of point %d/%d...", j+1, numOfSamples, i+1, numOfPoints);
			fflush(stdout);

			saveBufferFromSpi(&buf, &sz);
			chsz = sz/3;
			ch0 = buf;
			ch1 = &buf[chsz];
			ch2 = &buf[chsz*2];
			performThreshold(ch0, ch1, ch2, chsz, &samples[j*3], &samples[j*3+1], &samples[j*3+2]);

			printf("done\n");
			printf("%d %d %d\n", samples[j*3], samples[j*3+1], samples[j*3+2]);
			//plotGraph(ch0, ch1, ch2, chsz);
			free(buf);
			sleep(1);
		}
		int index = getBestSample(samples, numOfSamples, 3, CALIBRATION_MARGIN);
		calibrationData[i*3] = samples[index*3];
		calibrationData[i*3+1] = samples[index*3+1];
		calibrationData[i*3+2] = samples[index*3+2];
		free(samples);
	}

	writeArrayToFile(CALIBRATION_DATA_PATH, numOfPts, intPerPt, calibrationData);
}

int calculateLocation(int d0, int d1, int d2){
	int i=0;
	int least;
	int ret;
	for(;i<numOfPts;i++){
		int tmp;
		int errSqSum = 0;
		tmp = d0 - calibrationData[i*3];
		errSqSum += tmp*tmp;
		tmp = d1 - calibrationData[i*3+1];
		errSqSum += tmp*tmp;
		tmp = d2 - calibrationData[i*3+2];
		errSqSum += tmp*tmp;
		if(i){
			if(errSqSum < least){
				least = errSqSum;
				ret = i;
			}
		}else{
			least = errSqSum;
			ret = i;
		}
	}
	return ret;
}

