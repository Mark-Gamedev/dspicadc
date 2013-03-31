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

#define SMPSZ 500

#define STARTWORD 0xFABC
#define ENDWORD   0xFABD

extern int xcorr(int*, int*, int, double*);
extern int maxIndex(double *, int , double *);
static int ch0b0[SMPSZ];
static int ch1b0[SMPSZ];
static int ch0b1[SMPSZ];
static int ch1b1[SMPSZ];
static int fillingB1;
static int fillCounter;

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

void processData6(uint8_t *data){
	int val2 = data[5] << 8 | data[4];
	int val1 = data[3] << 8 | data[2];
	int val0 = data[1] << 8 | data[0];
	printf("%04x %04x %04x\n", val0, val1, val2);
}

void processData2(uint8_t *data){
	int raw = data[1] << 8 | data[0];
	int scanCounter = (raw >> 10) & 0b11;
	int sampleCounter = (raw >> 12) & 0xF;
	int value = raw & 0x3FF;
	//if(raw & 0x8000){
	//	printf("samples in buffer: %d  %04x\n", raw & 0x7FFF, raw & 0x7FFF);
	//}else{
		printf("%04d %01d %04d    %04x\n", sampleCounter, scanCounter, value, raw);
	//}
}

void processDataForXcorr(uint8_t *data){
	int raw = data[1] << 8 | data[0];
	int scanCounter = (raw >> 10) & 0b11;
	int value = raw & 0x3FF;

	int *ch0, *ch1;
	if(fillingB1){
		ch0 = (int*)ch0b1;
		ch1 = (int*)ch1b1;
	}else{
		ch0 = ch0b0;
		ch1 = ch1b0;
	}

	if(scanCounter==0){
		ch0[fillCounter >> 1] = value;
	}
	if(scanCounter==1){
		ch1[fillCounter >> 1] = value;
	}
	fillCounter++;
	if((fillCounter >> 1) == SMPSZ){
		//int d = xcorr(ch1, ch0, SMPSZ);
		//printf("%d\n", d);
		fillingB1 ^= 1;
		//pthread_create(&thread, 0, performXCorr, 0);
		fillCounter = 0;
	}
}

void processDataTimeDelay(short *data){
	int raw;

	raw = 0xFFFF & data[0];
	if((raw & 0xFF00)!=0xAB00){
		return;
	}
	//printf("%04x\t", raw);

	raw = 0xFFFF & data[1];
	printf("\t%d\n", raw);
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

	processDataTimeDelay((short*)rx);
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
#define NUMOFPOINTS 100
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

void performXCorr(){
	int *buf, *ch0, *ch1, index;
	int sz, chsz;
	double *corr, maxVal;
	saveBufferFromSpi(&buf, &sz);
	chsz = sz >> 1;
	ch0 = buf;
	ch1 = &buf[chsz];
	corr = calloc(sz, sizeof(double));
	xcorr(ch0, ch1, chsz, corr);
	index = maxIndex(corr, sz, &maxVal);
	index -= chsz;
	printVisual(index+512, 1024);
	printf("delay: %d, correlation: %lf\n", index, maxVal);
}

void saveWaveToFile(){
	int *buf;
	int sz;
	int i;
	for(i=0;i<10;i++){
		saveBufferFromSpi(&buf, &sz);
		if(sz==1024){
			printf("got one wave sample, saving...");
			fflush(stdout);
			char path[10];
			sprintf(path, "ch0_%d.txt", i);
			int fd0 = open(path, O_RDWR|O_CREAT);
			fchmod(fd0, 00664);
			sprintf(path, "ch1_%d.txt", i);
			int fd1 = open(path, O_RDWR|O_CREAT);
			fchmod(fd1, 00664);
			int j;
			for(j=0;j<512;j++){
				char data[5];
				sprintf(data, "%4d\n", buf[j]);
				write(fd0, data, 5);
				sprintf(data, "%4d\n", buf[j+512]);
				write(fd1, data, 5);
			}
			free(buf);
			close(fd0);
			close(fd1);
			printf("saved files for sample %d\n", i);
		}
	}
}

int main(int argc, char *argv[]){
	spiInit();

	//saveWave();
	while(1){
		performXCorr();
	}

	spiCleanup();
	return 0;
}
