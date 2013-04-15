#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

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

void plotWithGnuplot(char *fmt, ...){

	va_list ap;
	char* xs;
	int xi;
	FILE *fd;
	char cmd[1024];
	char path[1024];
	char data[1024];
	char *imgPath = PNGPATH;
	int i = 0;

	sprintf(cmd, "echo \"set term pngcairo; ");
	va_start(ap, fmt);
	while(*fmt){
		switch(*fmt){
			case 's':
				xs = va_arg(ap, char*);
				sprintf(cmd, "%s, plot \\\"%s\\\" with line", cmd, xs);
				break;
			case 'd':
				xi = va_arg(ap, int);

				sprintf(path, "%s-p%d", DATAPATH, i++);
				fd = fopen(path, "w");
				sprintf(data, "%d\t%d\n", xi-1, 0);
				sprintf(data, "%s%d\t%d\n", data, xi, 1024);
				fputs(data, fd);
				fclose(fd);

				sprintf(cmd, "%s, plot \\\"%s\\\" with line", cmd, path);

				break;
		}
		fmt++;
	}
	va_end(ap);
	sprintf(cmd, "%s | gnuplot > %s", cmd, imgPath);
	cmd[25] = ' ';

	printf("cmd:%s\n", cmd);exit(0);

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
	plotWithGnuplot("ss", path0, path1); 
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

	plotWithGnuplot("ssss", path0, path1, pathp0, pathp1);
}

