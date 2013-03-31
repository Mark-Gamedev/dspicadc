#include <stdio.h>

#define DATAPATH "/tmp/impact-temp-data"

void plotGraph(int *data, int size){
	saveBufferToFile(data, size, DATAPATH);
}

void saveBufferToFile(int *buf, int size, char* path){
	FILE *fd = fopen(path, "w");
	//fchmod(fd, 00664);
	int i;
	for(i=0;i<size;i++){
		char data[50];
		sprintf(data, "%d\n", buf[j]);
		fputs(data, fd);
	}
	close(fd);
}


