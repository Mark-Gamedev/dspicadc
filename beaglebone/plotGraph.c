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

	sprintf(cmd, "echo \"set term pngcairo; plot");
	va_start(ap, fmt);
	while(*fmt){
		switch(*fmt){
			case 's':
				xs = va_arg(ap, char*);
				sprintf(cmd, "%s, \\\"%s\\\" with line", cmd, xs);
				break;
			case 'd':
				xi = va_arg(ap, int);

				sprintf(path, "%s-p%d", DATAPATH, i++);
				fd = fopen(path, "w");
				sprintf(data, "%d\t%d\n", xi-1, 0);
				sprintf(data, "%s%d\t%d\n", data, xi, 2000);
				fputs(data, fd);
				fclose(fd);

				sprintf(cmd, "%s, \\\"%s\\\" with line", cmd, path);

				break;
		}
		fmt++;
	}
	va_end(ap);
	sprintf(cmd, "%s\" | gnuplot > %s", cmd, imgPath);
	cmd[29] = ' ';

	//printf("cmd:%s\n", cmd);exit(0);

	system(cmd);
	sprintf(cmd, "feh -Z -F %s", imgPath);
	system(cmd);
}

void plotGraph(int *ch0, int *ch1, int *ch2, int chsz){
	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");
	plotWithGnuplot("sss", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2");
}

