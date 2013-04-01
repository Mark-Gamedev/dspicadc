#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATAPATH "/tmp/impact-temp-data"
#define PNGPATH "/tmp/impact-temp-png"

void saveBufferToFile(int *buf, int size, char* path){
	FILE *fd = fopen(path, "w");
	int i;
	for(i=0;i<size;i++){
		char data[50];
		sprintf(data, "%d\n", buf[i]);
		fputs(data, fd);
	}
	fclose(fd);
}

void plotWithGnuplot(char *path0, char* path1, char *imagePath){
	char *imgPath = imagePath;
	if(!imgPath){
		imgPath = PNGPATH;
	}
	char cmd[1024];
	sprintf(cmd, "echo \"set term pngcairo; \
		   plot \\\"%s\\\" with line lt 3, \\\"%s\\\" with line lt 1; \
		   \" | gnuplot > %s", path0, path1, imagePath);
	printf("%s\n", cmd);
	system(cmd);
	sprintf(cmd, "feh -Z -F %s", imgPath);
	system(cmd);
}

void plotGraph(int *data0, int *data1, int size){
	char path0[256];
	char path1[256];

	sprintf(path0, "%s-%d", DATAPATH, 0);
	saveBufferToFile(data0, size, path0);
	sprintf(path1, "%s-%d", DATAPATH, 1);
	saveBufferToFile(data1, size, path1);
	plotWithGnuplot(path0, path1, 0);
}

