#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int readArray(char *path, int *outx, int *outy, int **array){
	FILE *fd;
	char buf[1024];
	int x, y;
	int i, j, k;
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
}

