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
#include <pthread.h>

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
#define THRESHOLD 500
extern int firstPeak(int*, int, int);
extern int maxIndex(double *, int , double *);
extern void plotWithGnuplot(char *, ...);

static void pabort(const char *s);
int saveBufferFromSpi(int**, int*);

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
}

void performXCorr(){
	int *buf, *ch0, *ch1, *ch2, index;
	int sz, chsz;
	double *corr, maxVal;
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
	printf("0-1 delay: %d, corr: %lf", index, maxVal);
	xcorr(ch0, ch2, chsz, corr);
	index = maxIndex(corr, sz, &maxVal);
	index -= chsz;
	printf("\t0-2 delay: %d, corr: %lf", index, maxVal);
	xcorr(ch1, ch2, chsz, corr);
	index = maxIndex(corr, sz, &maxVal);
	index -= chsz;
	printf("\t1-2 delay: %d, corr: %lf\n", index, maxVal);
	//printVisual(index+SMPSZ, SMPSZ*2);
}

int main(int argc, char *argv[]){
	spiInit();

	while(1){
		//performXCorr();
		performFirstPeak();
	}

	spiCleanup();
	return 0;
}
