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

void plotWithGnuplot(char *path0, char* path1, char* path2, char* path3, char *imagePath){
	char *imgPath = imagePath;
	if(!imgPath){
		imgPath = PNGPATH;
	}
	char cmd[1024];
	if(path2 && path3){
		sprintf(cmd, "echo \"set term pngcairo; \
				plot \\\"%s\\\" with line lt 3, \
				\\\"%s\\\" with line lt 1, \
				\\\"%s\\\" with line lt 4, \
				\\\"%s\\\" with line lt 2; \
				\" | gnuplot > %s", path0, path1, path2, path3, imgPath);
	}else{
		sprintf(cmd, "echo \"set term pngcairo; \
				plot \\\"%s\\\" with line lt 3, \\\"%s\\\" with line lt 1; \
				\" | gnuplot > %s", path0, path1, imgPath);
	}
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
	plotWithGnuplot(path0, path1, 0, 0, 0);
}

void plotGraphAndLines(int *data0, int *data1, int index0, int index1, int size){
	char path0[128];
	char path1[128];
	char pathp0[128];
	char pathp1[128];
	char data[128];
	FILE *fd;

	sprintf(path0, "%s-%d", DATAPATH, 0);
	saveBufferToFile(data0, size, path0);
	sprintf(path1, "%s-%d", DATAPATH, 1);
	saveBufferToFile(data1, size, path1);

	sprintf(pathp0, "%s-p%d", DATAPATH, 0);
	fd = fopen(pathp0, "w");
	sprintf(data, "%d\t%d\n", index0-1, 0);
	sprintf(data, "%s%d\t%d\n", data, index0, 1024);
	fputs(data, fd);
	fclose(fd);

	sprintf(pathp1, "%s-p%d", DATAPATH, 1);
	fd = fopen(pathp1, "w");
	sprintf(data, "%d\t%d\n", index1-1, 0);
	sprintf(data, "%s%d\t%d\n", data, index1, 1024);
	fputs(data, fd);
	fclose(fd);

	plotWithGnuplot(path0, path1, pathp0, pathp1, 0);
}

