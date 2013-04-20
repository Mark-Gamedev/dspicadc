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
#include <linux/spi/spidev.h>

static const char *device = "/dev/spidev2.0";
static uint8_t mode = 0;
static uint8_t bits = 16;
static uint32_t speed = 1000000;
static uint16_t delay = 0;
static int fd;

// max is 158
//#define SZ 150
#define SZ 2

#define SMPSZ 512

#define STARTWORD 0xFABC

extern int xcorr(int*, int*, int, double*);
#define THRESHOLD 580
extern int firstPeak(int*, int, int);
extern int maxIndex(double *, int , double *);
extern void plotWithGnuplot(char *, ...);
extern void saveBufferToFile(int *buf, int size, char* path);

extern void startServer();
extern void sendToServer(char *data, int len);

static void pabort(const char *s);
int saveBufferFromSpi(int**, int*);
void calculateCoord(int tl, int tr, int bl);

int hexDumpCount;
void hexDump(int x){
	if(!x){
		return;
	}
	if(x==STARTWORD){
		printf("#### ");
	}else{
		printf("%04x ", x);
	}
	hexDumpCount++;
	if((hexDumpCount & 15) == 0){
		printf("\n");
	}
}

int spiGetInt16(){
	uint8_t tx[2] = {0x00, };
	uint8_t rx[2] = {0x00, };

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = 2,
		.delay_usecs = delay,
	};

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1){
		pabort("can't send spi message");
	}
	ret = *((int *)(&rx[0])) & 0xFFFF;
	//hexDump(ret);
	return ret;
}

void spiInit(){
	int ret = 0;

	fd = open(device, O_RDWR);
	if (fd < 0){
		pabort("can't open device");
	}
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1){
		pabort("can't set mode");
	}

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1){
		pabort("can't get mode");
	}

	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1){
		pabort("can't set bit per word");
	}
	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1){
		pabort("can't get bit per word");
	}

	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		pabort("can't set max speed hz");
	}

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1){
		pabort("can't get max speed hz");
	}
}

void spiCleanup(){
	close(fd);
}

static void pabort(const char *s){
	perror(s);
	abort();
}

void printDataPoint(short *data, int size){
	int i = data[0] & 0xFFFF;
	if(i&0x8000){
		if((i & 0xFFED) == 0xFFED){
			exit(0);
		}
		if((i & 0xFBC0) == 0xFBC0){
			printf("########\n");
		}else{
			i &= 0x7FFF;
			printf("%d\n", i);
		}
	}
}

int transfer(){
	uint8_t tx[SZ] = {0x00, };
	uint8_t rx[SZ] = {0x00, };

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = SZ,
		.delay_usecs = delay,
		//.cs_change = 1,
		//.speed_hz = 0,
		//.bits_per_word = 0,
	};

	int ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret == 1){
		pabort("can't send spi message");
	}

	//processDataTimeDelay((short*)rx);
	//printDataPoint((short*)rx, SZ>>1);

	return 0;
}

int saveBufferFromSpi(int** outPtr, int* outCount){
	int word, count, *buffer, i;

	if(!outPtr){
		return -1;
	}
	
	while(word != STARTWORD){
		word = spiGetInt16();
	}
	count = spiGetInt16();
	buffer = calloc(count, sizeof(int));

	for(i=0;i<count;i++){
		buffer[i] = spiGetInt16();
	}

	*outPtr = buffer;
	if(outCount){
		*outCount = count;
	}
	return count;
}

void printVisual(int x, int max){
#define NUMOFPOINTS 102
       int count = (int)((float)x/max*NUMOFPOINTS);
       int i;
       for(i=0;i<NUMOFPOINTS;i++){
               if(i==count){
                       printf("#");
               }else{
                       printf("-");
               }
               //printf("\n");
       }
}

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
	plotWithGnuplot("sss", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2");
}

void performXCorr(){
	int *buf, *ch0, *ch1, *ch2, index;
	int sz, chsz;
	double *corr, maxVal;
	int msg[3];
	printf("waiting for knock...\n");
	saveBufferFromSpi(&buf, &sz);
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];
	corr = calloc(chsz*2, sizeof(double));

	xcorr(ch0, ch1, chsz, corr);
	index = maxIndex(corr, sz, &maxVal);
	index -= chsz;
	msg[0] = index;
	printf("0-1 delay: %d, corr: %lf", index, maxVal);
	xcorr(ch0, ch2, chsz, corr);
	index = maxIndex(corr, sz, &maxVal);
	index -= chsz;
	msg[1] = index;
	printf("\t0-2 delay: %d, corr: %lf", index, maxVal);
	xcorr(ch1, ch2, chsz, corr);
	index = maxIndex(corr, sz, &maxVal);
	index -= chsz;
	msg[2] = index;
	msg[0] = msg[0];
	printf("\t1-2 delay: %d, corr: %lf\n", index, maxVal);

	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");
	//sendToServer((char*)msg, sizeof(msg));
	plotWithGnuplot("sss", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2");
}

void performThreshold(){
	int *buf, *ch0, *ch1, *ch2;
	int index0, index1, index2;
	int sz, chsz;
	int i;
	int msg[3];

	index0 = index1 = index2 = 0;

	printf("waiting for knock...\n");

	saveBufferFromSpi(&buf, &sz);
	chsz = sz/3;
	ch0 = buf;
	ch1 = &buf[chsz];
	ch2 = &buf[chsz*2];

	for(i=0;i<chsz;i++){
		if(ch0[i] > THRESHOLD){
			index0 = i;
			break;
		}
	}
	
	for(i=0;i<chsz;i++){
		if(ch1[i] > THRESHOLD){
			index1 = i;
			break;
		}
	}
	
	for(i=0;i<chsz;i++){
		if(ch2[i] > THRESHOLD){
			index2 = i;
			break;
		}
	}

	if(!index0 || !index1 || !index2){
		return;
	}

	memset(msg, 0, sizeof(msg));
	msg[0] = index1 - index0;
	msg[1] = index2 - index0;
	msg[2] = index2 - index1;

	printf("%d  %d  %d \n", index0, index1, index2);
	saveBufferToFile(ch0, chsz, "/tmp/ch0");
	saveBufferToFile(ch1, chsz, "/tmp/ch1");
	saveBufferToFile(ch2, chsz, "/tmp/ch2");
	calculateCoord(index2, index0, index1);
	sendToServer((char*)msg, sizeof(msg));
	//plotWithGnuplot("sssddd", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2", index0, index1, index2);
	//plotWithGnuplot("sss", "/tmp/ch0", "/tmp/ch1", "/tmp/ch2", index0, index1, index2);
}

void calculateCoord(int tl, int tr, int bl){
	int x = tr - tl;
	int y = bl - tl;
	int i, j;
	int scale = 2;
	int offset = 30;
	printf("%d, %d\n", x, y);
	return;
	x+=offset/2;
	y+=offset/2;
	for(i=0; i<offset; i++){
		for(j=0; j<offset; j++){
			if(x==i*scale&&y==j*scale){
				printf("#");
			}else if(i==0 || i==offset-1){
				printf("-");
			}else if(j==0 || j==offset-1){
				printf("|");
			}else{
				printf(" ");
			}
		}
		printf("\n");
	}
}

int main(int argc, char *argv[]){
	printf("connecting to SPI...");
	fflush(stdout);
	spiInit();
	printf("done\n");

	printf("waiting for TCP connection...");
	fflush(stdout);
	//startServer();
	printf("done\n");

	while(1){
		//performXCorr();
		performThreshold();
		//performFirstPeak();
	}

	spiCleanup();
	return 0;
}

