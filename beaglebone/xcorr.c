#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#define START 0
#define END   512
#define SIZE  (END-START)

void xcorr(int *x, int *y, int n, double *out){
	int i,j;
	double mx,my,sx,sy,sxy,denom,r;
	int maxdelay = 10;

	/* Calculate the mean of the two series x[], y[] */
	mx = 0;
	my = 0;   
	for (i=0;i<n;i++) {
		mx += x[i];
		my += y[i];
	}
	mx /= n;
	my /= n;

	/* Calculate the denominator */
	sx = 0;
	sy = 0;
	for (i=0;i<n;i++) {
		sx += (x[i] - mx) * (x[i] - mx);
		sy += (y[i] - my) * (y[i] - my);
	}
	denom = sqrt(sx*sy);

	/* Calculate the correlation series */
	int delay;
	for (delay=-maxdelay;delay<maxdelay;delay++) {
		sxy = 0;
		for (i=0;i<n;i++) {
			j = i + delay;
			if (j < 0 || j >= n)
				continue;
			else
				sxy += (x[i] - mx) * (y[j] - my);
			/* Or should it be (?)
			   if (j < 0 || j >= n)
			   sxy += (x[i] - mx) * (-my);
			   else
			   sxy += (x[i] - mx) * (y[j] - my);
			   */
		}
		r = sxy / denom;
		out[delay+maxdelay] = r;
		/* r is the correlation coefficient at "delay" */
	}
}

void loadFileToArray(char *path, int* out){
	char buf[10];
	FILE *fd = fopen(path, "r");
	if(fd < 0){
		return;
	}
	int i = 0;
	for(i=0;i<END;i++){
		fgets(buf, 10, fd);
		if(i>=START){
			out[i-START] = atoi(buf);
		}
	}
	fclose(fd);
}

int maxIndex(double *src, int size, double *value){
	double max = 0-1;
	int i, index;
	for(i=0;i<size;i++){
		if(src[i] > max){
			max = src[i];
			index = i;
		}
	}
	if(value){
		*value = max;
	}
	return index;
}

void perform(int num){
	int ch0[SIZE];
	int ch1[SIZE];
	double result[SIZE*2];
	char path[20];
	sprintf(path, "sampleData/ch0_%d.txt", num);
	loadFileToArray(path, ch0);
	sprintf(path, "sampleData/ch1_%d.txt", num);
	loadFileToArray(path, ch1);
	xcorr(ch0, ch1, SIZE, result);
	double val;
	int i = maxIndex(result, SIZE*2, &val);
	printf("%d : [%d] = %lf\n", num, i+START, val);
}

